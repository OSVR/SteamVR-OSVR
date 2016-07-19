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

//
// getCanonicalPath(path)
//

TEST(getCanonicalPath_noBasePath, requiresAbsolutePath)
{
    // Input paths that relative are forbidden
    EXPECT_THROW(getCanonicalPath("relative/path"), std::invalid_argument);
}

TEST(getCanonicalPath_noBasePath, canonicalInputUnchanged)
{
    // Input paths that are already canonical should remain unchanged
    EXPECT_EQ("/me", getCanonicalPath("/me"));
}

TEST(getCanonicalPath_noBasePath, removeDoubleSlashes)
{
    // Double slashes are removed
    EXPECT_EQ("/me", getCanonicalPath("//me"));
    EXPECT_EQ("/me/head", getCanonicalPath("//me//head"));
}

TEST(getCanonicalPath_noBasePath, removeSingleDot)
{
    // Single dots are removed
    EXPECT_EQ("/me/head", getCanonicalPath("/me/./head"));
    EXPECT_EQ("/me/head", getCanonicalPath("/me/./././head"));
}

TEST(getCanonicalPath_noBasePath, removeDoubleDot)
{
    // Double dots are resolved
    EXPECT_EQ("/head", getCanonicalPath("/me/../head"));
    EXPECT_EQ("/me", getCanonicalPath("/me/head/.."));
    EXPECT_EQ("/me/", getCanonicalPath("/me/head/../"));
    EXPECT_EQ("/me/head", getCanonicalPath("/me/../me/head/eyes/left/../.."));
}

TEST(getCanonicalPath_noBasePath, tooManyDoubleDots)
{
    // Trying to get to a parent of the root is illegal
    EXPECT_THROW(getCanonicalPath("/root/../../"), std::invalid_argument);
}

//
// getCanonicalPath(path, base_path)
//

TEST(getCanonicalPath_withBasePath, requiresAbsolutePath)
{
    // Relative base paths are illegal
    EXPECT_THROW(getCanonicalPath("relative/path", "relative/path"), std::invalid_argument);
    EXPECT_THROW(getCanonicalPath("/absolute/path", "relative/path"), std::invalid_argument);
}

TEST(getCanonicalPath_withBasePath, canonicalInputUnchanged)
{
    EXPECT_EQ("/me", getCanonicalPath("/me", ""));
    EXPECT_EQ("/me", getCanonicalPath("/me", "/"));
}

TEST(getCanonicalPath_withBasePath, removeDoubleSlashes)
{
    EXPECT_EQ("/me/head", getCanonicalPath("//head", "/me"));
    EXPECT_EQ("/me/head", getCanonicalPath("/head", "/me/"));
}

TEST(getCanonicalPath_withBasePath, tooManyDoubleDots)
{
    EXPECT_THROW(getCanonicalPath("../../", "/root"), std::invalid_argument);
}

TEST(appendPath, emptyPrefix)
{
    EXPECT_EQ("/me", appendPath("", "/me"));
}

TEST(appendPath, emptySuffix)
{
    EXPECT_EQ("/me", appendPath("/me", ""));
}

TEST(appendPath, addsSlash)
{
    EXPECT_EQ("/me/head", appendPath("/me", "head"));
}

TEST(appendPath, removesDoubleSlashes)
{
    EXPECT_EQ("/me/head", appendPath("/me/", "/head"));
}

TEST(resolvePath, emptyBasePathThrowsException)
{
    EXPECT_THROW(resolvePath("/me", ""), std::invalid_argument);
}

TEST(resolvePath, relativeBasePathThrowsException)
{
    EXPECT_THROW(resolvePath("/me", "relative"), std::invalid_argument);
}

TEST(resolvePath, emptyPathReturnsBasePath)
{
    EXPECT_EQ("/me", resolvePath("", "/me"));
}

TEST(resolvePath, absolutePathReturnsPath)
{
    EXPECT_EQ("/me/head", resolvePath("/me/head", "/"));
}

TEST(resolvePath, appendsAndReturnsCanonicalPath)
{
    EXPECT_EQ("/me/head", resolvePath("/me/head", "/"));
    EXPECT_EQ("/me/head", resolvePath("head", "/me"));
    EXPECT_EQ("/me/hands/left", resolvePath("left", "/me/hands"));
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}

