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
imgexp.cpp - imgexp types
*/

#include "imgexp.h"
#include <json/json.h>

using namespace std;

namespace imgexp {

///////////////////////////////////////////////////////////////////////////////
//// Exception
///////////////////////////////////////////////////////////////////////////////
Exception::Exception(const char *file, const char *compilationTime, const char *function, const int line, ErrorCode code, const string message)
    : _message(message), _code(code), _line(line), _file(file), _compilationTime(compilationTime), _function(function)
{}

///////////////////////////////////////////////////////////////////////////////
//// helpers
///////////////////////////////////////////////////////////////////////////////
const Json::Value GetOrThrow(const Json::Value &value, const char *key)
{
    static const Json::Value Null;
    auto ret = value.get(key, Null);
    if (ret == Null)
        ThrowKeyNotFound(key);

    return ret;
}
void RequireTypeName(const Json::Value &value, const std::string &cmpType)
{
    auto type = GetOrThrow(value, "type").asString();
    if (type.empty() || type != cmpType)
        ThrowInvalidType;
}
Operand *CreateOperand(const Json::Value &value)
{
    auto type = GetOrThrow(value, "type").asString();

    static std::string ExactPixelMatchStr("ExactPixelMatch");
    static std::string RangePixelMatchStr("RangePixelMatch");
    static std::string ExpressionStr("Expression");

    if (type == ExpressionStr)
        return new Expression(value);
    else if (type == ExactPixelMatchStr)
        return new ExactPixelMatch(value);
    else if (type == RangePixelMatchStr)
        return new RangePixelMatch(value);

    ThrowInvalidType;
}

std::string GetAllText(const std::string &fileName)
{
    std::ifstream t(fileName);
    std::string str;

    t.seekg(0, std::ios::end);
    str.reserve(t.tellg());
    t.seekg(0, std::ios::beg);

    str.assign((std::istreambuf_iterator<char>(t)),
                std::istreambuf_iterator<char>());

    return str;
}

///////////////////////////////////////////////////////////////////////////////
//// Size
///////////////////////////////////////////////////////////////////////////////
Size::Size()
{}
Size::Size(unsigned long width, unsigned long height)
    : _width(width), _height(height), _area(width * height)
{}
Size::Size(const Json::Value &value)
{
    _height = GetOrThrow(value, "height").asUInt();
    _width = GetOrThrow(value, "width").asUInt();
    _area = _height * _width;
}
Size::operator const Json::Value() const
{
    Json::Value value;
    value["height"] = (Json::Value::LargestUInt) _height;
    value["width"] = (Json::Value::LargestUInt) _width;
    value["type"] = "Size";
    return value;
}
///////////////////////////////////////////////////////////////////////////////
//// Area
///////////////////////////////////////////////////////////////////////////////
Area::Area(long left, long top, long right, long bottom)
    : Area(Point(left, top), Point(right, bottom))
{}
Area::Area(const Point &topLeft, const Point &bottomRight)
    : _topLeft(topLeft), _bottomRight(bottomRight)
{
    if (topLeft <= bottomRight)
    {
        _topLeft = topLeft;
        _bottomRight = bottomRight;
    }
    else
        ThrowArgument("topLeft must be <= bottomRight");
}
Area::Area(const Json::Value &value)
{
    RequireTypeName(value, "Area");

    _topLeft = Point(GetOrThrow(value, "topLeft"));
    _bottomRight = Point(GetOrThrow(value, "bottomRight"));
}
Area::operator const Json::Value() const
{
    Json::Value value;
    value["topLeft"] = _topLeft;
    value["bottomRight"] = _bottomRight;
    value["type"] = "Area";
    return value;
}
///////////////////////////////////////////////////////////////////////////////
//// Point
///////////////////////////////////////////////////////////////////////////////
Point::Point()
{}
Point::Point(const Point &rhs)
{
    _x = rhs._x;
    _y = rhs._y;
}
Point::Point(const long x, const long y)
    : _x(x), _y(y)
{}
Point::Point(const Json::Value &value)
{
    RequireTypeName(value, "Point");

    _x = static_cast<long>(GetOrThrow(value, "x").asInt64());
    _y = static_cast<long>(GetOrThrow(value, "y").asInt64());
}
Point::operator const Json::Value() const
{
    Json::Value value;
    value["y"] = static_cast<Json::Value::Int64>(_y);
    value["x"] = static_cast<Json::Value::Int64>(_x);
    value["type"] = "Point";
    return value;
}
///////////////////////////////////////////////////////////////////////////////
//// Color
///////////////////////////////////////////////////////////////////////////////
Color::Color()
{}
Color::Color(BYTE red, BYTE green, BYTE blue)
    : _red(red), _green(green), _blue(blue)
{}
Color::Color(const Json::Value &value)
{
    RequireTypeName(value, "Color");

    _blue = GetOrThrow(value, "blue").asUInt();
    _green = GetOrThrow(value, "green").asUInt();
    _red = GetOrThrow(value, "red").asUInt();
}
Color::operator const Json::Value() const
{
    Json::Value value;
    value["blue"] = _blue;
    value["green"] = _green;
    value["red"] = _red;
    value["type"] = "Color";
    return value;
}
///////////////////////////////////////////////////////////////////////////////
//// Bitmap
///////////////////////////////////////////////////////////////////////////////
Bitmap *Bitmap::FromFile(const string &fileName)
{
    //open the file
    auto file = CreateFile(fileName.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
                           OPEN_EXISTING, 0, 0);

    if (file == INVALID_HANDLE_VALUE)
        ThrowFileNotFound(fileName);

    //read the headers
    DWORD read;
    BITMAPFILEHEADER bmfh;
    if (!ReadFile(file, &bmfh, sizeof(BITMAPFILEHEADER), &read, NULL))
        ThrowIORead("unable to read the bitmap file header");

    BITMAPINFOHEADER bmih;
    if (!ReadFile(file, &bmih, sizeof(BITMAPINFOHEADER), &read, NULL))
        ThrowIORead("unable to read the bitmap info header");

    if (bmih.biHeight < 0 || bmih.biWidth < 0)
        ThrowIORead("invalid dimensions in bitmap info header");

    auto colors = new Color[bmih.biHeight * bmih.biWidth];

    //read the colors
    if (!ReadFile(file, colors, (bmih.biHeight * bmih.biWidth) * sizeof(Color), &read, NULL))
        ThrowIORead("unable to read the color values");

    CloseHandle(file);

    return new Bitmap(bmih, colors);
}
Bitmap::Bitmap(const BITMAPINFOHEADER &bitmapInfo, const Color colors[])
    : _colors(colors), _bitmapInfo(bitmapInfo), _width(bitmapInfo.biWidth), _height(bitmapInfo.biHeight)
{}

Bitmap::~Bitmap()
{
    if (_colors)
        delete[] _colors;
}

void Bitmap::Save(const string &fileName) const
{
    // Create the .BMP file.
    auto file = CreateFile(fileName.c_str(), GENERIC_READ | GENERIC_WRITE, (DWORD)0,
                           NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);

    if (file != INVALID_HANDLE_VALUE)
        ThrowIOWrite("unable to create the bitmap file");

    //create the file header
    BITMAPFILEHEADER fh;
    fh.bfType = 0x4d42;
    fh.bfSize = sizeof(BITMAPFILEHEADER)+_bitmapInfo.biSize +
            _bitmapInfo.biClrUsed * sizeof(Color)+_bitmapInfo.biSizeImage;
    fh.bfReserved1 = 0;
    fh.bfReserved2 = 0;

    fh.bfOffBits = sizeof(BITMAPFILEHEADER)+
            _bitmapInfo.biSize + _bitmapInfo.biClrUsed * sizeof(Color);

    DWORD dwTmp;
    //write the bitmap file header to the file
    if (WriteFile(file, (LPVOID)&fh, sizeof(BITMAPFILEHEADER), &dwTmp, 0))
        ThrowIOWrite("unable to write the BITMAPFILEHEADER");

    //write the bitmap info header to the file
    if (!WriteFile(file, (LPVOID)&_bitmapInfo, sizeof(BITMAPINFOHEADER)+_bitmapInfo.biClrUsed * sizeof(Color), &dwTmp, 0))
        ThrowIOWrite("unable to write the BITMAPINFOHEADER");

    //write the color bytes
    if (!WriteFile(file, _colors, _bitmapInfo.biSizeImage, &dwTmp, 0))
        ThrowIOWrite("unable to write the colors");

    if (!CloseHandle(file))
        ThrowIOWrite("unable to close the file handle");
}

///////////////////////////////////////////////////////////////////////////////
//// Operand
///////////////////////////////////////////////////////////////////////////////
Operand::Operand()
{}
///////////////////////////////////////////////////////////////////////////////
//// PixelMatch
///////////////////////////////////////////////////////////////////////////////
PixelMatch::PixelMatch(const Json::Value &value)
{
    //no type name requirement because this is a protected ctor
    _offset = Point(value);
}
PixelMatch::PixelMatch()
{}
///////////////////////////////////////////////////////////////////////////////
//// ExactPixelMatch
///////////////////////////////////////////////////////////////////////////////
ExactPixelMatch::ExactPixelMatch(const _Color &color)
    : _color(color)
{}
bool ExactPixelMatch::Eval(const Bitmap &ss, const Point &start) const
{
    return ss.getColor(start + _offset) == _color;
}
ExactPixelMatch::ExactPixelMatch(const Json::Value &value)
{
    RequireTypeName(value, "ExactPixelMatch");
    _offset = Point(GetOrThrow(value, "offset"));
    _color = _Color(GetOrThrow(value, "color"));
}
ExactPixelMatch::operator const Json::Value() const
{
    Json::Value value;
    value["color"] = _color;
    value["offset"] = _offset;
    value["type"] = "ExactPixelMatch";
    return value;
}
///////////////////////////////////////////////////////////////////////////////
//// RangePixelMatch
///////////////////////////////////////////////////////////////////////////////
RangePixelMatch::RangePixelMatch(const Color &min, const Color &max)
    : _min(min), _max(max)
{}
bool RangePixelMatch::Eval(const Bitmap &ss, const Point &start) const
{
    auto color = ss.getColor(start + _offset);
    return color >= _min && color <= _max;
}
RangePixelMatch::RangePixelMatch(const Json::Value &value)
{
    RequireTypeName(value, "RangePixelMatch");

    _min = Color(GetOrThrow(value, "min"));
    _max = Color(GetOrThrow(value, "max"));
}
RangePixelMatch::operator const Json::Value() const
{
    Json::Value value;
    value["min"] = _min;
    value["max"] = _max;
    value["offset"] = _offset;
    value["type"] = "RangePixelMatch";
    return value;
}
///////////////////////////////////////////////////////////////////////////////
//// Expression
///////////////////////////////////////////////////////////////////////////////
Expression::Expression(Operand *left, ::imgexp::Operator op, Operand *right)
    : _left(left), _operator(op), _right(right)
{}
bool Expression::Eval(const Bitmap &ss, const Point &start) const
{
    switch (_operator)
    {
    case ::imgexp::Operator::OR:
        return _left->Eval(ss, start) || _right->Eval(ss, start);
        break;
    case ::imgexp::Operator::XOR:
        return _left->Eval(ss, start) ^ _right->Eval(ss, start);
        break;
    case ::imgexp::Operator::AND:
        return _left->Eval(ss, start) && _right->Eval(ss, start);
        break;
    default:
    case ::imgexp::Operator::NONE:
        return _left->Eval(ss, start);
        break;
    }
}
Expression::~Expression()
{
    if (_left)
        delete _left;

    if (_right)
        delete _right;
}
Expression::Expression(const Json::Value &value)
{
    RequireTypeName(value, "Expression");

    _left = CreateOperand(GetOrThrow(value, "left"));

    auto opNode = value["operator"];
    auto rightNode = value["right"];

    if (!opNode.isNull())
    {
        _operator = StringToOperator(opNode.asString());

        if (!rightNode.isNull())
        {
            if (_operator == ::imgexp::Operator::NONE)
                ThrowDeserialization("right not cannot exist with a NONE operator");

            _right = CreateOperand(rightNode);
        }
        else if (_operator != ::imgexp::Operator::NONE)
            ThrowDeserialization("right operand cannot exist with a NONE operator");
    }
    else if (!rightNode.isNull())
        ThrowDeserialization("right operand cannot exist without an operator");

}
Expression::operator const Json::Value() const
{
    Json::Value value;
    value["left"] = *_left;
    value["operator"] = OperatorToString(_operator);
    value["right"] = *_right;
    value["type"] = "Expression";
    return value;
}
///////////////////////////////////////////////////////////////////////////////
//// PixelPattern
///////////////////////////////////////////////////////////////////////////////
const wchar_t* PixelPattern::PIXEL_PATTERN_FILE_EXT = L".pattern";
void PixelPattern::Reset()
{
    if (_found)
    {
        delete _found;
        _found = nullptr;
    }

    _changed = false;
}
void PixelPattern::Update(const Bitmap &ss)
{
    auto sz = ss.Size();

    if (sz != _imageSize)
        ThrowLogic(format("invalid image size %1%/%2%", % sz.Width() % sz.Height()));

    _changed = false;
    bool wasFound = _found != nullptr;

    if (_found)
    {
        //found in the same place as last time, hasn't changed
        if (_root->Eval(ss, *_found))
            return;

        //clear it out
        delete _found;
        _found = nullptr;
    }

    long height = ss.Height();
    long width = ss.Width();

    if (_flagMatrix)
    {
        auto &fm = *_flagMatrix;

        for (long y = 0; y < height; ++y)
        {
            for (long x = 0; x < width; ++x)
            {
                if (fm[x][y])
                {
                    Point pt(x, y);
                    if (_root->Eval(ss, pt))
                    {
                        _found = new Point(pt);
                        _changed = true;
                        return;
                    }
                }
            }
        }
    }
    else
    {
        for (long y = 0; y < height; ++y)
        {
            for (long x = 0; x < width; ++x)
            {
                Point pt(x, y);
                if (_root->Eval(ss, pt))
                {
                    _found = new Point(pt);
                    _changed = true;
                    return;
                }
            }
        }
    }

    if (wasFound)
        _changed = true;
}
FlagMatrix *PixelPattern::CreateFlagMatrix(const Size &imageSize, const std::vector<Area> &searchAreas)
{
    auto flagMatrix = new std::vector<std::vector<bool>>(imageSize.Width());
    for (auto &dim : *flagMatrix)
        dim.resize(imageSize.Height());

    auto &fm = *flagMatrix;

    for (auto &area : searchAreas)
    {
        for (long y = area.Top(); y != area.Bottom() + 1; ++y)
        {
            for (long x = area.Left(); x != area.Right() + 1; ++x)
            {
                if (!fm[x][y])
                    fm[x][y] = true;
            }
        }
    }

    return flagMatrix;
}
PixelPattern::PixelPattern(Size imageSize, PatternId id, Expression &root, std::vector<Area> *searchAreas)
    :_imageSize(imageSize), _id(id), _root(&root),
      _flagMatrix(searchAreas ? CreateFlagMatrix(imageSize, *searchAreas) : nullptr),
      _searchAreas(searchAreas)
{}
PixelPattern::~PixelPattern()
{
    if (_root)
        delete _root;

    if (_flagMatrix)
        delete _flagMatrix;

    if (_searchAreas)
        delete _searchAreas;

    if (_found)
        delete _found;
}
PixelPattern *PixelPattern::FromFile(const string &file)
{
    //loads the PixelPattern from the supplied file
    Json::Value patternNode;
    Json::Reader reader;

    string contents = GetAllText(file);

    if (!reader.parse(contents, patternNode, false))
        ThrowDeserialization(format("unable to parse json from file %1%: %2%", % file % reader.getFormattedErrorMessages()));

    return new PixelPattern(patternNode);
}
PixelPattern::PixelPattern(const Json::Value &value)
{
    RequireTypeName(value, "PixelPattern");

    _id = static_cast<PatternId>(GetOrThrow(value, "id").asUInt64());
    _root = dynamic_cast<Expression*>(CreateOperand(GetOrThrow(value, "root")));
    _imageSize = Size(GetOrThrow(value, "imageSize"));

    auto searchAreasNode = GetOrThrow(value, "searchAreas");
    auto searchAreas = new std::vector<Area>;
    try
    {
        for (unsigned i = 0; i < searchAreasNode.size(); ++i)
            searchAreas->push_back(Area(searchAreasNode[i]));
    }
    catch (...)
    {
        delete searchAreas;
        throw;
    }

    _flagMatrix = searchAreas ? CreateFlagMatrix(_imageSize, *searchAreas) : nullptr;
    _searchAreas = searchAreas;
}
PixelPattern::operator const Json::Value() const
{
    Json::Value value;
    value["type"] = "PixelPattern";
    //JsonCpp doesn't have an unsigned long type so I've got to convert to a unsigned long long
    value["id"] = static_cast<unsigned long long>(_id);
    value["root"] = *_root;
    value["imageSize"] = _imageSize;
    auto &searchAreas = *_searchAreas;
    for (unsigned i = 0; i < searchAreas.size(); ++i)
        value["searchAreas"][i] = searchAreas[i];
    //searchPoints isn't serialized as it can be recreated from searchAreas
    return value;
}

///////////////////////////////////////////////////////////////////////////////
//// Parser
///////////////////////////////////////////////////////////////////////////////
Parser::Parser(const Size &imageSize)
    : _imageSize(imageSize)
{
    if (imageSize.Width() <= 0)
        ThrowArgument("imageSize.Width must be > 0");

    if (imageSize.Height() <= 0)
        ThrowArgument("imageSize.Height must be > 0");
}
Parser::~Parser()
{
    if (_patterns)
    {
        for (auto it : *_patterns)
        {
            if (it.second)
                delete it.second;
        }

        delete _patterns;
    }
}
void Parser::AddPattern(const PixelPattern &pattern)
{
    if (_patterns->count(pattern.Id()) > 0)
        ThrowDuplicateKey(std::to_string(pattern.Id()));

    _patterns->operator[](pattern.Id()) = new PixelPattern(pattern);
}
void Parser::RemovePattern(PatternId id)
{
    _patterns->erase(id);
}
const PixelPattern *Parser::GetPattern(PatternId id) const
{
    PatternMap::const_iterator found = _patterns->find(id);
    if (found != _patterns->end())
        return found->second;
    else
        return nullptr;
}
void Parser::_Parse(const Bitmap &bmp, bool reset)
{
    for (auto &pattern : *_patterns)
    {
        if (reset)
            pattern.second->Reset();

        pattern.second->Update(bmp);
    }
}
///////////////////////////////////////////////////////////////////////////////
//// SingleParser
///////////////////////////////////////////////////////////////////////////////
SingleParser::SingleParser(const Size &imageSize)
    : Parser(imageSize)
{}
void SingleParser::Parse(const Bitmap &bmp)
{
    Parser::_Parse(bmp, true);
}

///////////////////////////////////////////////////////////////////////////////
//// SeriesParser
///////////////////////////////////////////////////////////////////////////////
SeriesParser::SeriesParser(const Size &imageSize) : Parser(imageSize)
{}
void SeriesParser::Next(const Bitmap &bmp, bool reset)
{
    Parser::_Parse(bmp, reset);
}

}
