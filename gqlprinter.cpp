/** \file
 * \brief dummy GQL connector mainly used for testing and debugging.
 *
 * it returns the query in GQL format and hence is bascially a no-op.
 * It cannot connect to a database and does not return any data
 *
 * \author Claudio Fleiner
 * \copyright 2018 Claudio Fleiner
 *
 * **License:** 
 *
 * > This program is free software: you can redistribute it and/or modify
 * > it under the terms of the GNU Affero General Public License as published by
 * > the Free Software Foundation, either version 3 of the License, or
 * > (at your option) any later version.
 * >
 * > This program is distributed in the hope that it will be useful,
 * > but WITHOUT ANY WARRANTY; without even the implied warranty of
 * > MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * > GNU Affero General Public License for more details.
 * >
 * > You should have received a copy of the GNU Affero General Public License
 * > along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <cstdint>
#include <unordered_map>
#include <exception>
#include <jsoncpp/json/json.h>
#include <boost/algorithm/string.hpp>
#include "libgqlsql.h"


std::string GQL_SQL::GQLParser::ParserGQL::target() const { return "GQL"; }
GQL_SQL::GQLParser::ParserGQL::~ParserGQL() { }

void GQL_SQL::GQLParser::ParserGQL::createResult()
{
    if(!query_) { return; }
    res_.result=query_->to_string();
}
