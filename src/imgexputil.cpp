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
util.cpp - imgexp utility classes
*/

#include "imgexputil.h"

IMGEXPUTIL_NS_START

Expression *BuildExpressionTree(const std::map<Point, PixelMatch*> &matchMap)
{
	Expression *root, *prev, *leaf;
	root = prev = leaf = nullptr;

	Point rootPt(matchMap.begin()->first);

	for (auto pair : matchMap)
	{
		auto &point = pair.first;
		auto match = pair.second;
		match->Offset(point - rootPt);

		leaf = new Expression(match);

		if (prev) //attach the previous to this one on the right
			prev->SetRight(Operator::AND, leaf);

		prev = leaf;

		if (!root)
			root = leaf;
	}

	return root;
}

IMGEXPUTIL_NS_END