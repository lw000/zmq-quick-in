#include "utils.h"

std::ostream& operator << (std::ostream& out, const msg::ItemValue& value)
{
    out << value.n() << " "
        << value.nid() << " "
        << value.t() << " "
        << value.q() << " "
        << value.dt() << " ";
    switch (value.dt())
    {
    case msg::BOOL:
        out << value.v().b();
    case msg::CHAR:
        out << value.v().c();
    case msg::BYTE:
        out << value.v().bt();
    case msg::INT16:
        out << value.v().i16();
    case msg::UINT16:
        out << value.v().u16();
    case msg::INT32:
        out << value.v().i32();
    case msg::UINT32:
        out << value.v().i32();
    case msg::INT64:
        out << value.v().i64();
    case msg::UINT64:
        out << value.v().u64();
    case msg::FLOAT32:
        out << value.v().f32();
    case msg::FLOAT64:
        out << value.v().f64();
    case  msg::STRING:
        out << value.v().str();
    case  msg::STREAM:
        out << value.v().stream();
    case  msg::BLOB:
        out << value.v().blob();
    default:
        break;
    }
    return out;
}

std::ostream& operator << (std::ostream& out, const msg::BlockItemValue& values)
{
    int idx = 1;
    for (auto& value : values.values())
    {
        out << "[" << values.idx() << "] "
            << idx++ << " "
            << value << std::endl;
    }

    return out;
}