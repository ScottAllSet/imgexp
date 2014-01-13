/*
Copyright 2013 Scott R. Jones

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
===============================================================================
util.h - imgexp utility classes
*/

#ifndef _IMGEXPUTIL_H_
#define _IMGEXPUTIL_H_

#include <map>
#include "imgexp.h"

#define IMGEXPUTIL_NS_START namespace imgexp { namespace util {
#define IMGEXPUTIL_NS_END }}

IMGEXPUTIL_NS_START

Expression *BuildExpressionTree(const std::map<Point, PixelMatch*> &matchMap);

IMGEXPUTIL_NS_END

#endif //_IMGEXPUTIL_H_