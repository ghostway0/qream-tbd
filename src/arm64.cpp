#include <cstdint>
#include <vector>
#include <absl/log/log.h>
#include <absl/status/statusor.h>
#include <sys/mman.h>

#include "qream/ir.h"
#include "qream/match.h"
#include "qream/utils.h"

using OutputIt = std::back_insert_iterator<std::vector<uint8_t>>;

constexpr const size_t kInstructionSize = 4;
constexpr const size_t kMemPoolSize = 1024;

namespace {

inline void emit_ldst_imm(uint32_t opcode, uint32_t rt, uint32_t rn,
                          uint32_t imm12, OutputIt &out) {
  // [31:30] size (11 = 64-bit)
  // [29:28] opc (depends on LDR/STR)
  // [27:26] 01 (for load/store immediate unsigned offset)
  // [25]    must be 0
  // [24:5]  imm12 << scale
  // [4:0]   Rt (target/source register)
  uint32_t instr = (opcode << 22) | (imm12 << 10) | (rn << 5) | rt;
  out++ = instr & 0xFF;
  out++ = (instr >> 8) & 0xFF;
  out++ = (instr >> 16) & 0xFF;
  out++ = (instr >> 24) & 0xFF;
}

inline void emit_3reg(uint32_t opcode21, uint32_t rd, uint32_t rn,
                      uint32_t rm, OutputIt &out) {
  // [31:21] opcode [20:16] Rm [15:10] unused [9:5] Rn [4:0] Rd
  uint32_t instr = (opcode21 << 21) | (rm << 16) | (rn << 5) | rd;
  out++ = instr & 0xFF;
  out++ = (instr >> 8) & 0xFF;
  out++ = (instr >> 16) & 0xFF;
  out++ = (instr >> 24) & 0xFF;
}

using MMU = uint64_t (*)(uint64_t);

struct SegmentFlags {
  bool cachable : 1;
  bool read_only : 1;
  bool executable : 1;
  bool device_mapped : 1;
};

struct AllocatedSegment {
  uint64_t guest_base;
  void *mem;
  size_t length;
  SegmentFlags flags;

  bool contains(uint64_t guest_addr) const {
    return guest_addr >= guest_base && guest_addr < guest_base + length;
  }

  bool is_cachable() const { return flags.cachable; }

  uint64_t to_host(uint64_t guest_addr) const {
    return reinterpret_cast<uint64_t>(mem) + (guest_addr - guest_base);
  }
};

struct ValuePool {
  AllocatedSegment underlying_;
  uint64_t current_offset = 0;

  ValuePool(AllocatedSegment underlying) : underlying_(underlying) {}

  uint64_t addValue(std::span<const uint8_t> value) {
    std::memcpy(static_cast<uint8_t *>(underlying_.mem) + current_offset,
                value.data(), value.size());
    uint64_t off = current_offset;
    current_offset += value.size();
    return off;
  }

  uint64_t addValue(uint64_t value) {
    current_offset = (current_offset + 8) & 0xFFFFFFFFFFFFFFF8;
    std::memcpy(static_cast<uint8_t *>(underlying_.mem) + current_offset,
                &value, sizeof(value));
    uint64_t off = current_offset;
    current_offset += sizeof(value);
    return off;
  }

  uint64_t guestBase() { return underlying_.guest_base; }
};

struct GuestMemory {
  MMU resolve;
  std::vector<AllocatedSegment> segments;

  const AllocatedSegment *get_segment(uint64_t guest_addr) const {
    for (const auto &seg : segments) {
      if (seg.contains(guest_addr)) {
        return &seg;
      }
    }
    return nullptr;
  }

  AllocatedSegment mapInto(uint64_t guest_base, size_t length,
                           SegmentFlags flags) {
    size_t page_size = static_cast<size_t>(sysconf(_SC_PAGESIZE));
    size_t aligned_length = (length + page_size - 1) & ~(page_size - 1);

    int prot = PROT_READ | PROT_WRITE;
    if (flags.read_only) {
      prot = PROT_READ;
    }
    if (flags.executable) {
      prot |= PROT_EXEC;
    }

    void *mem = mmap(nullptr, aligned_length, prot,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    assert(mem != MAP_FAILED);

    AllocatedSegment seg{.guest_base = guest_base,
                         .mem = mem,
                         .length = aligned_length,
                         .flags = flags};
    segments.push_back(seg);
    return seg;
  }

  std::optional<uint64_t> resolve_static(uint64_t guest_addr) const {
    const AllocatedSegment *seg = get_segment(guest_addr);
    if (seg && seg->is_cachable()) {
      return seg->to_host(guest_addr);
    }
    return std::nullopt;
  }
};

struct Env {
  uint64_t pc;
  GuestMemory mem;
  ValuePool const_pool;

  Env(uint64_t entrypoint)
      : pc(entrypoint), mem{}, const_pool(mem.mapInto(pc, kMemPoolSize)) {}
};

struct OpEmitter {
  OutputIt &out;
  Env &env;

  absl::Status emit_binary_op(const Operation &op, uint32_t encoding) {
    return MATCH_OP(Int64, Scalar, Register, Register, Register)(
        op,
        [encoding](const Operation &, const Register &dst,
                   const Register &lhs, const Register &rhs,
                   OutputIt &out) {
          emit_3reg(encoding, dst.enc, lhs.enc, rhs.enc, out);
        },
        out);
  }

  absl::Status emit_unary_op(const Operation &op, uint32_t encoding,
                             uint8_t fixed_reg = 31) {
    return MATCH_OP(Int64, Scalar, Register, Register)(
        op,
        [encoding, fixed_reg](const Operation &, const Register &dst,
                              const Register &src, OutputIt &out) {
          emit_3reg(encoding, dst.enc, fixed_reg, src.enc, out);
        },
        out);
  }

  absl::Status emit_ldr(const Operation &op) {
    // LDR with register: LDR Rt, =imm64 (literal pool load)
    return MATCH_OP(Int64, Scalar, Imm64, Register)(
        op,
        [](const Operation &, const Imm64 &imm, const Register &rt,
           OutputIt &out) {
          uint32_t pool_offset = 0;
          emit_ldst_imm(0b0101100, rt.enc, 31 /* PC */, pool_offset, out);
        },
        out);
  }

  absl::Status emit_str(const Operation &op) {
    // should call MMU helper here
    RETURN_IF_OK(MATCH_OP(Int64, Scalar, MemoryAddressing, Register)(
        op,
        [this](const Operation &, const MemoryAddressing &mem,
               const Register &rt, OutputIt &out) {
          uint32_t imm12 = mem.offset / 8;

          emit_ldst_imm(0b1111100100, rt.enc, mem.base_reg->enc, imm12,
                        out);
          env.pc += 4;
        },
        out));

    // STR with register: STR Rt, [Xn, #imm]
    RETURN_IF_OK(MATCH_OP(Int64, Scalar, Imm64, Register)(
        op,
        [this](const Operation &, const Imm64 &imm, const Register &rt,
               OutputIt &out) {
          ValuePool &pool = env.const_pool;

          uint64_t imm_offset =
              (pool.addValue(imm) + pool.guestBase() - env.pc) / 4;

          if (imm_offset < -1048576 || imm_offset > 1048575) {
            return absl::OutOfRangeError("LDR literal offset out of range");
          }

          uint32_t instr =
              0x58000000 | ((imm_offset & 0x1FFFFF) << 5) | rt.enc;

          out++ = instr & 0xFF;
          out++ = (instr >> 8) & 0xFF;
          out++ = (instr >> 16) & 0xFF;
          out++ = (instr >> 24) & 0xFF;

          env.pc += 4;
          return absl::OkStatus();
        },
        out));

    return absl::OkStatus();
  }

  absl::Status try_emit(const Operation &op) {
    switch (op.irop) {
      case IROp::Add:
        return emit_binary_op(op, 0b10001011000);
      case IROp::Sub:
        return emit_binary_op(op, 0b11001011000);
      case IROp::Mul:
        return emit_binary_op(op, 0b10011011000);
      case IROp::And:
        return emit_binary_op(op, 0b10001010000);
      case IROp::Or:
        return emit_binary_op(op, 0b10101010000);
      case IROp::Xor:
        return emit_binary_op(op, 0b11001010000);
      case IROp::Neg:
        return emit_unary_op(op, 0b11001011000);
      case IROp::Ldr:
        return emit_ldr(op);
      case IROp::Str:
        return emit_str(op);
      default:
        return absl::InternalError(
            absl::StrFormat("%s not implemented", op.toString()));
    }
  }
};

} // namespace

absl::StatusOr<std::vector<uint8_t>> transpile_to_arm64(
    const std::vector<Operation> &ops) {
  std::vector<uint8_t> code;
  OutputIt out = std::back_inserter(code);

  Env env{0, GuestMemory{}};
  OpEmitter emitter{out, env};

  for (const Operation &op : ops) {
    TRY(emitter.try_emit(op));
  }

  return code;
}
