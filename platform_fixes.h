/** @file
    @brief Platform-specific fixes can go here

    @date 2016

    @author
    Sensics, Inc.
    <http://sensics.com/osvr>
*/

// Copyright 2016 Sensics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//        http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef INCLUDED_platform_fixes_h_GUID_8460694E_5363_48BD_3CEA_32F9E785B0E1
#define INCLUDED_platform_fixes_h_GUID_8460694E_5363_48BD_3CEA_32F9E785B0E1


// Internal Includes
// - none

// Library/third-party includes
// - none

// Standard includes
// - none

// http://stackoverflow.com/questions/3694723/error-c3861-strcasecmp-identifier-not-found-in-visual-studio-2008
#ifdef _MSC_VER 
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#endif

#endif // INCLUDED_platform_fixes_h_GUID_8460694E_5363_48BD_3CEA_32F9E785B0E1

