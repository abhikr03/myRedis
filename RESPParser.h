#pragma once
#include "common.h"

#define NULL_BULK_STRING "NULL"
#define READ_CACHE_MAX 8192
#define ITEM_LEN_MAX 536870912 

class RESPParser {

private:
    int _read_fd = -1; 
    std::string _read_cache = "";


protected:
    bool _validate_array_size(const std::string& size_item);
    bool _validate_bstr_size(const std::string& size_item);
    bool _validate_crlf(const std::string& bstr);
    bool _cache_has_valid_item(std::string& item);
    void _update_cache();
    std::string read_from_fd(int n_bytes);
    std::string read_next_item();


public:
    RESPParser(int fd) {
        _read_fd = fd;
        _read_cache = "";
    }
    std::vector<std::string> read_new_request();
};