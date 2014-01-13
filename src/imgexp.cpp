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

IMGEXP_NS_START

///////////////////////////////////////////////////////////////////////////////
//// Exception
///////////////////////////////////////////////////////////////////////////////
Exception::Exception(const char *file, const char *compilationTime, const char *function, const int line, ErrorCode code, const string message)
: _message(message), _code(code), _line(line), _file(file), _compilationTime(compilationTime), _function(function),
domain_error(message)
{}

///////////////////////////////////////////////////////////////////////////////
//// helpers
///////////////////////////////////////////////////////////////////////////////
template <class T>
static bool VectorsEqual(const vector<T> &a, const vector<T> &b)
{
	//copied from http://stackoverflow.com/questions/17394149/how-to-efficiently-compare-vectors-with-c

	if (a.size() != b.size())
		return false;

	const size_t n = a.size(); // make it const and unsigned!
	std::vector<bool> free(n, true);

	for (size_t i = 0; i < n; ++i)
	{
		bool matchFound = false;
		auto start = b.cbegin();

		while (true)
		{
			const auto position = std::find(start, b.cend(), a[i]);

			if (position == b.cend())
				break; // nothing found

			const auto index = position - b.cbegin();
			if (free[index])
			{
				// free pair found
				free[index] = false;
				matchFound = true;
				break;
			}
			else
				start = position + 1; // search in the rest
		}

		if (!matchFound)
			return false;
	}
	return true;
}
void WriteJsonToFile(const string &file, const Json::Value &value)
{
	Json::StyledWriter writer;
	string jsonStr = writer.write(value);
	ofstream output(file, ios_base::trunc);
	output << jsonStr;
	output.close();
}
Json::Value ParseJsonFromFile(const string &file)
{
	Json::Value node;
	Json::Reader reader;

	string contents = GetAllText(file);

	if (!reader.parse(contents, node, false))
		ThrowJsonRead(format("error parsing json from file %1%: %2%", % file % reader.getFormattedErrorMessages()));

	return node;
}
const Json::Value GetJsonValue(const Json::Value &value, const char *key, bool throwOnFailure = true)
{
	auto ret = value.get(key, Json::Value::null);
	if (ret.isNull() && throwOnFailure)
		ThrowKeyNotFound(key);

	return ret;
}
void RequireTypeName(const Json::Value &value, const std::string &cmpType)
{
	auto type = GetJsonValue(value, "type").asString();
	if (type.empty() || type != cmpType)
		ThrowInvalidType;
}
Operand *CreateOperand(const Json::Value &value, const char *key)
{
	auto operandValue = value.get(key, Json::Value::null);
	if (!operandValue.isNull())
	{
		auto type = GetJsonValue(operandValue, "type").asString();

		static std::string ExactPixelMatchStr("ExactPixelMatch");
		static std::string RangePixelMatchStr("RangePixelMatch");
		static std::string ExpressionStr("Expression");

		if (type == ExpressionStr)
			return new Expression(operandValue);
		else if (type == ExactPixelMatchStr)
			return new ExactPixelMatch(operandValue);
		else if (type == RangePixelMatchStr)
			return new RangePixelMatch(operandValue);
	}

	return nullptr;
}

void WriteAllText(const std::string &fileName, const std::string &text, bool append)
{
	std::ofstream output(fileName, append ? ios_base::app : ios_base::trunc);
	output << text;
	output.close();
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
	_height = GetJsonValue(value, "height").asUInt();
	_width = GetJsonValue(value, "width").asUInt();
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

unsigned long Size::Height() const
{
	return _height;
}

unsigned long Size::Width() const
{
	return _width;
}

unsigned long Size::Area() const
{
	return _area;
}

bool Size::operator==(const Size &rhs) const
{
	return _height == rhs._height && _width == rhs._width;
}

bool Size::operator!=(const Size &rhs) const
{
	return _height != rhs._height || _width != rhs._width;
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

	_topLeft = Point(GetJsonValue(value, "topLeft"));
	_bottomRight = Point(GetJsonValue(value, "bottomRight"));
}
Area::operator const Json::Value() const
{
	Json::Value value;
	value["topLeft"] = _topLeft;
	value["bottomRight"] = _bottomRight;
	value["type"] = "Area";
	return value;
}

Point Area::TopLeft() const
{
	return _topLeft;
}

Point Area::BottomRight() const
{
	return _bottomRight;
}

long Area::Left() const
{
	return _topLeft.X();
}

long Area::Top() const
{
	return _topLeft.Y();
}

long Area::Right() const
{
	return _bottomRight.X();
}

long Area::Bottom() const
{
	return _bottomRight.Y();
}

long Area::Height() const
{
	return Bottom() - Top() + 1;
}

long Area::Width() const
{
	return Right() - Left() + 1;
}

Area * Area::GetOverlappedArea(const Area &other) const
{
	Point topLeft;
	Point bottomRight;

	bool found = false;

	if (Contains(other._topLeft))
	{
		topLeft = other._topLeft;
		found = true;
	}
	else
		topLeft = _topLeft;

	if (Contains(other._bottomRight))
	{
		bottomRight = other._bottomRight;
		found = true;
	}
	else
		bottomRight = _bottomRight;

	return found ? new Area(topLeft, bottomRight) : nullptr;
}

bool Area::Overlaps(const Area &other) const
{
	return Contains(other._topLeft) || Contains(other._bottomRight);
}

bool Area::Contains(const Area &other) const
{
	return Contains(other._topLeft) && Contains(other._bottomRight);
}

bool Area::Contains(const Point &pt) const
{
	return pt.X() >= Left() && pt.X() <= Right() &&
		pt.Y() >= Top() && pt.Y() <= Bottom();
}

Area & Area::operator=(const Area &rhs)
{
	if (&rhs != this)
	{
		_topLeft = rhs._topLeft;
		_bottomRight = rhs._bottomRight;
	}

	return *this;
}

bool Area::operator==(const Area &rhs) const
{
	return Left() == rhs.Left() && Top() == rhs.Top() &&
		Right() == rhs.Right() && Bottom() == rhs.Bottom();
}

bool Area::operator!=(const Area &rhs) const
{
	return !operator==(rhs);
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

	_x = static_cast<long>(GetJsonValue(value, "x").asInt64());
	_y = static_cast<long>(GetJsonValue(value, "y").asInt64());
}
Point::operator const Json::Value() const
{
	Json::Value value;
	value["y"] = static_cast<Json::Value::Int64>(_y);
	value["x"] = static_cast<Json::Value::Int64>(_x);
	value["type"] = "Point";
	return value;
}

long Point::X() const
{
	return _x;
}

long Point::Y() const
{
	return _y;
}

void Point::Set(long x, long y)
{
	_x = x;
	_y = y;
}

Point & Point::operator=(const Point &rhs)
{
	if (&rhs != this)
	{
		_x = rhs._x;
		_y = rhs._y;
	}

	return *this;
}
Point &Point::operator+=(const Point &rhs)
{
	_x += rhs._x;
	_y += rhs._y;
	return *this;
}
bool Point::operator==(const Point &rhs) const
{
	return _x == rhs._x && _y == rhs._y;
}
Point Point::operator + (const Point &rhs) const
{
	return Point(_x + rhs._x, _y + rhs._y);
}
Point Point::operator-(const Point &rhs) const
{
	return Point(_x - rhs._x, _y - rhs._y);
}

bool Point::operator!=(const Point &rhs) const
{
	return !operator==(rhs);
}

bool Point::operator<(const Point &rhs) const
{
	return _y == rhs._y ? _x < rhs._x : _y < rhs._y;
}

bool Point::operator>(const Point &rhs) const
{
	return  rhs < *this;
}

bool Point::operator<=(const Point &rhs) const
{
	return !operator>(rhs);
}

bool Point::operator>=(const Point &rhs) const
{
	return !operator< (rhs);
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

	_blue = GetJsonValue(value, "blue").asUInt();
	_green = GetJsonValue(value, "green").asUInt();
	_red = GetJsonValue(value, "red").asUInt();
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

BYTE Color::Blue() const
{
	return _blue;
}

BYTE Color::Red() const
{
	return _red;
}

BYTE Color::Green() const
{
	return _green;
}

bool Color::operator==(const Color &rhs) const
{
	return rhs._blue == _blue && rhs._green == _green && rhs._red == _red;
}

bool Color::operator!=(const Color &rhs) const
{
	return !operator==(rhs);
}

bool Color::operator>(const Color &rhs) const
{
	return _blue > rhs._blue && _green > rhs._green && _red > rhs._red;
}

bool Color::operator<(const Color &rhs) const
{
	return _blue < rhs._blue && _green < rhs._green && _red < rhs._red;
}

bool Color::operator>=(const Color &rhs) const
{
	return _blue >= rhs._blue && _green >= rhs._green && _red >= rhs._red;
}

bool Color::operator<=(const Color &rhs) const
{
	return _blue <= rhs._blue && _green <= rhs._green && _red <= rhs._red;
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

	auto colors = new _Color[bmih.biHeight * bmih.biWidth];

	//read the colors
	if (!ReadFile(file, colors, (bmih.biHeight * bmih.biWidth) * sizeof(_Color), &read, NULL))
		ThrowIORead("unable to read the color values");

	CloseHandle(file);

	return new Bitmap(bmih, colors);
}
Bitmap::Bitmap(const BITMAPINFOHEADER &bitmapInfo, const _Color colors[])
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
		_bitmapInfo.biClrUsed * sizeof(_Color)+_bitmapInfo.biSizeImage;
	fh.bfReserved1 = 0;
	fh.bfReserved2 = 0;

	fh.bfOffBits = sizeof(BITMAPFILEHEADER)+
		_bitmapInfo.biSize + _bitmapInfo.biClrUsed * sizeof(_Color);

	DWORD dwTmp;
	//write the bitmap file header to the file
	if (WriteFile(file, (LPVOID)&fh, sizeof(BITMAPFILEHEADER), &dwTmp, 0))
		ThrowIOWrite("unable to write the BITMAPFILEHEADER");

	//write the bitmap info header to the file
	if (!WriteFile(file, (LPVOID)&_bitmapInfo, sizeof(BITMAPINFOHEADER)+_bitmapInfo.biClrUsed * sizeof(_Color), &dwTmp, 0))
		ThrowIOWrite("unable to write the BITMAPINFOHEADER");

	//write the color bytes
	if (!WriteFile(file, _colors, _bitmapInfo.biSizeImage, &dwTmp, 0))
		ThrowIOWrite("unable to write the colors");

	if (!CloseHandle(file))
		ThrowIOWrite("unable to close the file handle");
}

::imgexp::Size Bitmap::Size() const
{
	return ::imgexp::Size(_width, _height);
}

long Bitmap::Width() const
{
	return _width;
}

long Bitmap::Height() const
{
	return _height;
}

const _Color & Bitmap::Color(const Point &location) const
{
	return Color(location.X(), location.Y());
}

const _Color & Bitmap::Color(const long x, const long y) const
{
	return _colors[(_height - 1 - y)*_width + x];
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

bool PixelMatch::Equals(const Operand &rhs) const
{
	if (auto p = dynamic_cast<PixelMatch const*>(&rhs))
	{
		return _offset == p->_offset;
	}
	else
		return false;
}

Point PixelMatch::Offset() const
{
	return _offset;
}

void PixelMatch::Offset(Point val)
{
	_offset = val;
}

///////////////////////////////////////////////////////////////////////////////
//// ExactPixelMatch
///////////////////////////////////////////////////////////////////////////////
ExactPixelMatch::ExactPixelMatch(const _Color &color)
: _color(color)
{}
bool ExactPixelMatch::Eval(const Bitmap &ss, const Point &start) const
{
	return ss.Color(start + _offset) == _color;
}
ExactPixelMatch::ExactPixelMatch(const Json::Value &value)
{
	RequireTypeName(value, "ExactPixelMatch");
	_offset = Point(GetJsonValue(value, "offset"));
	_color = _Color(GetJsonValue(value, "color"));
}
ExactPixelMatch::operator const Json::Value() const
{
	Json::Value value;
	value["color"] = _color;
	value["offset"] = _offset;
	value["type"] = "ExactPixelMatch";
	return value;
}

bool ExactPixelMatch::Equals(const Operand &rhs) const
{
	if (auto p = dynamic_cast<ExactPixelMatch const*>(&rhs))
	{
		return PixelMatch::Equals(rhs) && _color == p->_color;
	}
	else
		return false;
}

imgexp::_Color ExactPixelMatch::Color() const
{
	return _color;
}

void ExactPixelMatch::Color(imgexp::_Color val)
{
	_color = val;
}

///////////////////////////////////////////////////////////////////////////////
//// RangePixelMatch
///////////////////////////////////////////////////////////////////////////////
RangePixelMatch::RangePixelMatch(const Color &min, const Color &max)
: _min(min), _max(max)
{}
bool RangePixelMatch::Eval(const Bitmap &ss, const Point &start) const
{
	auto color = ss.Color(start + _offset);
	return color >= _min && color <= _max;
}
RangePixelMatch::RangePixelMatch(const Json::Value &value)
{
	RequireTypeName(value, "RangePixelMatch");

	_min = Color(GetJsonValue(value, "min"));
	_max = Color(GetJsonValue(value, "max"));
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

bool RangePixelMatch::Equals(const Operand &rhs) const
{
	if (auto p = dynamic_cast<RangePixelMatch const*>(&rhs))
	{
		return PixelMatch::Equals(rhs) && _min == p->_min && _max == p->_max;
	}
	else
		return false;
}

const Color & RangePixelMatch::Min()
{
	return _min;
}

const Color & RangePixelMatch::Max()
{
	return _max;
}

///////////////////////////////////////////////////////////////////////////////
//// Expression
///////////////////////////////////////////////////////////////////////////////
Expression::Expression(Operand *left, ::imgexp::Operator op, Operand *right)
: _left(left), _operator(op), _right(right)
{
	if (!_left)
		ThrowArgument("left is required");

	if (_operator == ::imgexp::Operator::NONE && _right)
		ThrowArgument("right cannot exist with NONE operator");

	if (_operator != ::imgexp::Operator::NONE && !_right)
		ThrowArgument("right must exist if operator is not NONE");
}
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
	delete _left;

	if (_right)
		delete _right;
}
Expression::Expression(const Json::Value &value)
{
	RequireTypeName(value, "Expression");

	_left = CreateOperand(value, "left");
	if (!_left)
		ThrowDeserialization("left operand was missing");

	auto opNode = value["operator"];
	_operator = opNode.isNull() ? ::imgexp::Operator::NONE : StringToOperator(opNode.asString());
	_right = CreateOperand(value, "right");

	if (_operator == ::imgexp::Operator::NONE && _right)
		ThrowDeserialization("right cannot exist with NONE operator");

	if (_operator != ::imgexp::Operator::NONE && !_right)
		ThrowDeserialization("right must exist if operator is not NONE");
}
Expression::operator const Json::Value() const
{
	Json::Value value;
	value["left"] = *_left;
	value["operator"] = OperatorToString(_operator);
	if (_right)
		value["right"] = *_right;
	value["type"] = "Expression";
	return value;
}
bool Expression::Equals(const Operand &rhs) const
{
	if (auto p = dynamic_cast<Expression const*>(&rhs))
	{
		bool left = (*_left) == (*p->_left);
		bool right = (_right && p->_right) ? (*_right) == (*p->_right) : _right == p->_right;
		bool op = _operator == p->_operator;
		return left && right && op;
	}
	else
		return false;
}
Operand * Expression::Left() const
{
	return _left;
}
imgexp::Operator Expression::Operator() const
{
	return _operator;
}
Operand * Expression::Right() const
{
	return _right;
}
void Expression::SetRight(imgexp::Operator op, Operand *right)
{
	if (op == ::imgexp::Operator::NONE && right)
		ThrowArgument("right cannot exist with NONE operator");

	if (op != ::imgexp::Operator::NONE && !right)
		ThrowArgument("right must exist if operator is not NONE");

	_operator = op;
	_right = right;
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
PixelPattern::PixelPattern(Size imageSize, PatternId id, Expression *root, std::vector<Area> *searchAreas)
:_imageSize(imageSize), _id(id), _root(root),
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
	return new PixelPattern(ParseJsonFromFile(file));
}
PixelPattern::PixelPattern(const Json::Value &value)
{
	RequireTypeName(value, "PixelPattern");

	_id = static_cast<PatternId>(GetJsonValue(value, "id").asUInt64());
	_root = dynamic_cast<Expression*>(CreateOperand(value, "root"));
	if (!_root)
		ThrowDeserialization("root is missing");

	_imageSize = Size(GetJsonValue(value, "imageSize"));

	auto searchAreasNode = GetJsonValue(value, "searchAreas", false);
	if (!searchAreasNode.isNull())
	{
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
}
PixelPattern::operator const Json::Value() const
{
	Json::Value value;
	value["type"] = "PixelPattern";
	//JsonCpp doesn't have an unsigned long type so I've got to convert to a unsigned long long
	value["id"] = static_cast<unsigned long long>(_id);
	value["root"] = *_root;
	value["imageSize"] = _imageSize;
	if (_searchAreas)
	{
		auto &searchAreas = *_searchAreas;
		for (unsigned i = 0; i < searchAreas.size(); ++i)
			value["searchAreas"][i] = searchAreas[i];
	}
	//searchPoints isn't serialized as it can be recreated from searchAreas
	return value;
}

bool PixelPattern::operator==(const PixelPattern &rhs) const
{
	bool equal = _id == rhs._id &&
		_imageSize == rhs._imageSize &&
		*_root == *rhs._root &&
		((!_searchAreas && !_searchAreas) ||
		(_searchAreas && rhs._searchAreas && VectorsEqual(*_searchAreas, *rhs._searchAreas)));

	return equal;
}

bool PixelPattern::operator!=(const PixelPattern &rhs) const
{
	return !operator==(rhs);
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

imgexp::Operator StringToOperator(const std::string &str)
{
	if (str == OpOrStr)
		return ::imgexp::Operator::OR;
	else if (str == OpAndStr)
		return ::imgexp::Operator::AND;
	else if (str == OpXorStr)
		return ::imgexp::Operator::XOR;
	else
		return ::imgexp::Operator::NONE;
}

std::string OperatorToString(Operator value)
{
	switch (value)
	{
	case Operator::OR:
		return OpOrStr;
	case Operator::XOR:
		return OpXorStr;
	case Operator::AND:
		return OpAndStr;
	default:
	case Operator::NONE:
		return OpNoneStr;
	}
}

IMGEXP_NS_END
