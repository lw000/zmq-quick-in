#pragma once

#include <iostream>

#include "protos/msg/msg.pb.h"

std::ostream& operator << (std::ostream& out, const msg::ItemValue& value);

std::ostream& operator << (std::ostream& out, const msg::BlockItemValue& values);