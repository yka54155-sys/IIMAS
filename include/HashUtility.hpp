#pragma once
#include"std.hpp"
#include <openssl/types.h>
#include"openssl/evp.h"
std::string sha256_hex(const std::string& input);

std::string hash_password(const std::string& password,const std::string& salt);


