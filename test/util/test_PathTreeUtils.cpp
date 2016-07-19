/** @file
    @brief Unit tests for functions in PathTreeUtil.h

    @date 2016

    @author
    Sensics, Inc.
    <http://sensics.com>

*/

// Copyright 2016 Sensics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// 	http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Internal Includes
#include "PathTreeUtil.h"

// Library/third-party includes
#include <gtest/gtest.h>
#include <json/json.h>

// Standard includes
// - none

TEST(getCanonicalPath, removeDoubleSlashes)
{
    const std::string input = "//me//head";
    const std::string expected = "/me/head";

    EXPECT_EQ(expected, getCanonicalPath(input));
}

#if 0
TEST(appendPath, )
{
}

TEST(resolvePath, )
{
}
#endif

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}

