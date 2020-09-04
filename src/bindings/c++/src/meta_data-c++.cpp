/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2020 Dmitry Baryshev

    The MIT License

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#include <cstdlib>
#include <cstring>

#include "sail-common.h"
#include "sail.h"
#include "sail-c++.h"

namespace sail
{

class SAIL_HIDDEN meta_data::pimpl
{
public:
    pimpl()
        : key(SAIL_META_DATA_UNKNOWN)
    {
    }

    SailMetaData key;
    std::string key_unknown;
    std::string value;
};

meta_data::meta_data()
    : d(new pimpl)
{
}

meta_data::meta_data(const meta_data &md)
    : meta_data()
{
    *this = md;
}

meta_data& meta_data::operator=(const meta_data &md)
{
    with_key(md.key())
        .with_key_unknown(md.key_unknown())
        .with_value(md.value());

    return *this;
}

meta_data::~meta_data()
{
    delete d;
}

SailMetaData meta_data::key() const
{
    return d->key;
}

std::string meta_data::key_unknown() const
{
    return d->key_unknown;
}

std::string meta_data::value() const
{
    return d->value;
}

meta_data& meta_data::with_key(SailMetaData key)
{
    d->key = key;
    return *this;
}

meta_data& meta_data::with_key_unknown(const std::string &key_unknown)
{
    d->key_unknown = key_unknown;
    return *this;
}

meta_data& meta_data::with_value(const std::string &value)
{
    d->value = value;
    return *this;
}

sail_status_t meta_data::meta_data_to_string(enum SailMetaData meta_data, const char **result) {

    SAIL_TRY(sail_meta_data_to_string(meta_data, result));

    return SAIL_OK;
}

sail_status_t meta_data::meta_data_from_string(const char *str, enum SailMetaData *result) {

    SAIL_TRY(sail_meta_data_from_string(str, result));

    return SAIL_OK;
}

meta_data::meta_data(const sail_meta_data_node *md)
    : meta_data()
{
    if (md == nullptr) {
        SAIL_LOG_DEBUG("NULL pointer has been passed to sail::meta_data(). The object is untouched");
        return;
    }

    with_key(md->key)
        .with_key_unknown(md->key_unknown)
        .with_value(md->value);
}

sail_status_t meta_data::to_sail_meta_data_node(sail_meta_data_node *md) const
{
    SAIL_CHECK_META_DATA_NODE_PTR(md);

    md->key = d->key;

    if (d->key == SAIL_META_DATA_UNKNOWN) {
        SAIL_TRY(sail_strdup(d->key_unknown.c_str(), &md->key_unknown));
    }

    SAIL_TRY(sail_strdup(d->value.c_str(), &md->value));

    return SAIL_OK;
}

}
