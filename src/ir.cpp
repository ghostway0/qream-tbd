#include <ostream>

#include "qream/ir.h"

std::ostream &operator<<(std::ostream &os, const Operation &op) {
  if (op.predicate) {
    os << "@" << *op.predicate << " ";
  }
  os << op.irop << "." << op.dtype;

  if (op.shape != VectorShape::Scalar) {
    os << "x" << static_cast<size_t>(op.shape);
  }

  os << " ";

  for (size_t i = 0; i < op.num_operands; ++i) {
    if (i > 0) {
      os << ", ";
    }
    os << op.operands[i];
  }
  return os;
}

std::ostream &operator<<(std::ostream &os, const ScalarDType &dtype) {
  switch (dtype) {
    case ScalarDType::Float32:
      os << "f32";
      break;
    case ScalarDType::Float64:
      os << "f64";
      break;
    case ScalarDType::Float16:
      os << "f16";
      break;
    case ScalarDType::Int8:
      os << "i16";
      break;
    case ScalarDType::Int16:
      os << "i16";
      break;
    case ScalarDType::Int32:
      os << "i32";
      break;
    case ScalarDType::Int64:
      os << "i64";
      break;
    default:
      os << "opaque";
      break;
  }
  return os;
}

std::ostream &operator<<(std::ostream &os, const Access &access) {
  std::ios::fmtflags orig_flags = os.flags();
  switch (access.space) {
    case Space::Register:
      os << "r" << access.where;
      break;
    case Space::Memory:
    case Space::Immediate:
      os << "0x" << std::hex << access.where;
      break;
    case Space::IO:
      os << "io" << access.where;
      break;
    case Space::Special:
      if (access.where == 0) {
        os << "ip";
      } else if (access.where == 1) {
        os << "sp";
      } else {
        os << "s" << access.where;
      }
      break;
    default:
      os << "unknown" << access.where;
  }
  os.flags(orig_flags);
  return os;
}

std::ostream &operator<<(std::ostream &os, const IROp &op) {
  switch (op) {
    case IROp::Add:
      os << "add";
      break;
    case IROp::Mul:
      os << "mul";
      break;
    case IROp::Ldr:
      os << "ldr";
      break;
    case IROp::Str:
      os << "str";
      break;
    case IROp::Sub:
      os << "sub";
      break;
    case IROp::Div:
      os << "div";
      break;
    case IROp::Mod:
      os << "mod";
      break;
    case IROp::Neg:
      os << "neg";
      break;
    case IROp::And:
      os << "and";
      break;
    case IROp::Or:
      os << "or";
      break;
    case IROp::Xor:
      os << "xor";
      break;
    case IROp::Not:
      os << "not";
      break;
    case IROp::Shl:
      os << "shl";
      break;
    case IROp::Shr:
      os << "shr";
      break;
    case IROp::Rol:
      os << "rol";
      break;
    case IROp::Ror:
      os << "ror";
      break;
    case IROp::Jump:
      os << "jump";
      break;
    case IROp::JumpIf:
      os << "jumpif";
      break;
    case IROp::Call:
      os << "call";
      break;
    case IROp::Ret:
      os << "ret";
      break;
    case IROp::Trap:
      os << "trap";
      break;
    case IROp::Halt:
      os << "halt";
      break;
    case IROp::FAdd:
      os << "fadd";
      break;
    case IROp::FSub:
      os << "fsub";
      break;
    case IROp::FMul:
      os << "fmul";
      break;
    case IROp::FDiv:
      os << "fdiv";
      break;
    case IROp::SignExtend:
      os << "sext";
      break;
    case IROp::ZeroExtend:
      os << "zext";
      break;
    case IROp::Truncate:
      os << "trunc";
      break;
    case IROp::Fence:
      os << "fence";
      break;
    case IROp::AtomicAdd:
      os << "atomic_add";
      break;
    case IROp::AtomicCmpXchg:
      os << "atomic_cmpxchg";
      break;
    case IROp::VShuffle:
      os << "vshuffle";
      break;
    case IROp::VBlend:
      os << "vblend";
      break;
    case IROp::VExtract:
      os << "vextract";
      break;
    case IROp::VInsert:
      os << "vinsert";
      break;
    case IROp::VReduceAdd:
      os << "vreduce.add";
      break;
    default:
      os << "unknown_op";
      break;
  }
  return os;
}
