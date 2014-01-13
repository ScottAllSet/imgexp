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
imgexp.h - imgexp types
*/

#ifndef _IMGEXP_H_
#define _IMGEXP_H_

#ifdef _WIN32
#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <json/json-forwards.h>
#include <memory>
#include <string>
#include <fstream>
#include <streambuf>
#include <unordered_map>
#include <boost/current_function.hpp>
#include <boost/format.hpp>

#define format(fmt,args) boost::str(boost::format(fmt) args)

#define XStringify(s) Stringify(s)
#define Stringify(s) #s

#ifndef MAX_PATH
#error Missing MAX_PATH
#endif

#define IMGEXP_NS_START namespace imgexp {
#define IMGEXP_NS_END }

IMGEXP_NS_START

enum class ErrorCode {
	Logic,
	IORead,
	IOWrite,
	FileNotFound,
	Argument,
	KeyNotFound,
	DuplicateKey,
	InvalidType,
	Deserialization,
	Serialization,
	JsonRead,
	JsonWrite,
};

class Exception : std::domain_error {
public:
	Exception(const char *file, const char *compilationTime, const char *function, const int line, ErrorCode code, const std::string message);
	inline const std::string File() const { return _file; }
	inline const std::string CompilationTime() const { return _compilationTime; }
	inline const std::string Function() const { return _function; }
	inline int Line() const { return _line; }
	inline ErrorCode Code() const { return _code; }
	inline const std::string Message() const { return _message; }
private:
	const std::string _file;
	const std::string _compilationTime;
	const std::string _function;
	const std::string _message;
	const int _line;
	const ErrorCode _code;
};

#define __Throw(code, data) throw Exception (__FILE__, __DATE__, BOOST_CURRENT_FUNCTION, __LINE__, code , data)
#define ThrowLogic(message) __Throw(ErrorCode::Logic, message)
#define ThrowIORead(message) __Throw(ErrorCode::IORead, message)
#define ThrowJsonRead(message) __Throw(ErrorCode::JsonRead, message)
#define ThrowJsonWrite(message) __Throw(ErrorCode::JsonWrite, message)
#define ThrowIOWrite(message) __Throw(ErrorCode::IOWrite, message)
#define ThrowArgument(message) __Throw(ErrorCode::Argument, message)
#define ThrowKeyNotFound(key) __Throw(ErrorCode::KeyNotFound, key)
#define ThrowDuplicateKey(key) __Throw(ErrorCode::DuplicateKey, key)
#define ThrowDeserialization(message) __Throw(ErrorCode::Deserialization, message)
#define ThrowSerialization(message) __Throw(ErrorCode::Serialization, message)
#define ThrowFileNotFound(fileName) __Throw(ErrorCode::FileNotFound, fileName)
#define ThrowInvalidType __Throw(ErrorCode::InvalidType, (char*) 0)

#pragma region helpers
void WriteJsonToFile(const std::string &file, const Json::Value &value);
Json::Value ParseJsonFromFile(const std::string &file);
std::string GetAllText(const std::string &fileName);
void WriteAllText(const std::string &fileName, const std::string &text, bool append = false);
void RequireTypeName(const Json::Value &value, const std::string &cmpType);
class Operand;
Operand *CreateOperand(const Json::Value &value, const char* key);
//Definitions to make a type persistable to/from Json.
//This should only be applied to concrete types.
#define JsonPersistableDef(type)\
	explicit type(const Json::Value &value); \
	operator const Json::Value() const
#define VJsonPersistableDef(type)\
	explicit type(const Json::Value &value); \
	virtual operator const Json::Value() const
#pragma endregion

#pragma region graphics
class Size {
	unsigned long _height = 0;
	unsigned long _width = 0;
	unsigned long _area = 0;
public:
	JsonPersistableDef(Size);
	Size();
	Size(unsigned long width, unsigned long height);
	unsigned long Height() const;
	unsigned long Width() const;
	unsigned long Area() const;
	bool operator==(const Size &rhs) const;
	bool operator!=(const Size &rhs) const;
};
class Point {
	long _x = 0;
	long _y = 0;
public:
	JsonPersistableDef(Point);
	long X() const;
	long Y() const;
	static const Point Empty;
	Point();
	Point(const Point &rhs);
	Point(long x, long y);
	void Set(long x, long y);
	Point &operator=(const Point &rhs);
	Point &operator+=(const Point &rhs);
	bool operator==(const Point &rhs) const;
	Point operator+(const Point &rhs) const;
	Point operator-(const Point &rhs) const;
	bool operator!=(const Point &rhs) const;
	bool operator< (const Point &rhs) const;
	bool operator>(const Point &rhs) const;
	bool operator<=(const Point &rhs) const;
	bool operator>=(const Point &rhs) const;
};
class Area {
	Point _topLeft;
	Point _bottomRight;
public:
	JsonPersistableDef(Area);
	Point TopLeft() const;
	Point BottomRight() const;
	long Left() const;
	long Top() const;
	long Right() const;
	long Bottom() const;
	long Height() const;
	long Width() const;
	Area(long left, long top, long right, long bottom);
	Area(const Point &topLeft, const Point &bottomRight);
	//Gets the area "other" overlaps this Area
	Area *GetOverlappedArea(const Area &other) const;
	bool Overlaps(const Area &other) const;
	bool Contains(const Area &other) const;
	bool Contains(const Point &pt) const;
	Area &operator=(const Area &rhs);
	bool operator==(const Area &rhs) const;
	bool operator!=(const Area &rhs) const;
};

#include <pshpack1.h>
class Color {
	BYTE _blue;
	BYTE _green;
	BYTE _red;
public:
	JsonPersistableDef(Color);
	Color();
	Color(BYTE red, BYTE green, BYTE blue);
	BYTE Blue() const;
	BYTE Red() const;
	BYTE Green() const;
	bool operator==(const Color &rhs) const;
	bool operator!=(const Color &rhs) const;
	bool operator>(const Color &rhs) const;
	bool operator<(const Color &rhs) const;
	bool operator>=(const Color &rhs) const;
	bool operator<=(const Color &rhs) const;
};
#include <poppack.h>

typedef ::imgexp::Color _Color;
class Bitmap {
	const Color *_colors;
	const BITMAPINFOHEADER _bitmapInfo;
	const long _width;
	const long _height;
public:
	::imgexp::Size Size() const;
	long Width() const;
	long Height() const;
	static Bitmap *FromFile(const std::string &fileName);
	Bitmap(const BITMAPINFOHEADER &bitmapInfo, const Color colors[]);
	//note: no need for a cctor because _colors is const.
	virtual ~Bitmap();
	void Save(const std::string &fileName) const;
	const _Color &Color(const Point &location) const;
	const _Color &Color(const long x, const long y) const;
};
#pragma endregion

#pragma region expression tree
class PixelPattern;

class Operand {
public:
	Operand();
	virtual ~Operand() {}
	VJsonPersistableDef(Operand) = 0;
	virtual bool Eval(const Bitmap &ss, const Point &start) const = 0;
	friend bool operator==(Operand const &lhs, Operand const &rhs) {
		return lhs.Equals(rhs);
	}
	friend bool operator!=(Operand const &lhs, Operand const &rhs) {
		return !lhs.Equals(rhs);
	}
protected:
	virtual bool Equals(const Operand &rhs) const = 0;
};

class PixelMatch : public Operand {
public:
	Point Offset() const;
	void Offset(Point val);
	PixelMatch();
	virtual bool Equals(const Operand &rhs) const;
	VJsonPersistableDef(PixelMatch) = 0;
	virtual bool Eval(const Bitmap &ss, const Point &start) const = 0;
protected:
	Point _offset;
};

//A pixel color matches exactly
typedef ::imgexp::Color _Color;
class ExactPixelMatch : public PixelMatch {
public:
	VJsonPersistableDef(ExactPixelMatch);
	ExactPixelMatch(const _Color &color);
	virtual bool Eval(const Bitmap &ss, const Point &start) const;
	_Color Color() const;
	void Color(imgexp::_Color val);
protected:
	virtual bool Equals(const Operand &rhs) const;
private:
	_Color _color;
};

//A pixel color matches within a range of colors
class RangePixelMatch : public PixelMatch {
public:
	VJsonPersistableDef(RangePixelMatch);
	RangePixelMatch(const Color &min, const Color &max);
	virtual bool Eval(const Bitmap &ss, const Point &start) const;
	const Color &Min();
	const Color &Max();
protected:
	virtual bool Equals(const Operand &rhs) const;
private:
	Color _min;
	Color _max;
};

static std::string OpOrStr("OR");
static std::string OpAndStr("AND");
static std::string OpXorStr("XOR");
static std::string OpNoneStr("NONE");

enum class Operator {
	NONE,
	OR,
	XOR,
	AND,
};

Operator StringToOperator(const std::string &str);
std::string OperatorToString(Operator value);

class Expression : public Operand {
	Operand * _left = nullptr;
	Operand * _right = nullptr;
	::imgexp::Operator _operator = ::imgexp::Operator::NONE;
public:
	Expression(Operand *left, ::imgexp::Operator op = ::imgexp::Operator::NONE, Operand *right = nullptr);
	virtual ~Expression();
	VJsonPersistableDef(Expression);
	Operand * Left() const;
	imgexp::Operator Operator() const;
	Operand * Right() const;
	void SetRight(::imgexp::Operator op = ::imgexp::Operator::NONE, Operand *right = nullptr);
	virtual bool Eval(const Bitmap &ss, const Point &start) const;
protected:
	virtual bool Equals(const Operand &rhs) const;
};

typedef std::vector<std::vector<bool>> FlagMatrix;
typedef unsigned long PatternId;
class PixelPattern {
	bool _changed = false;
	PatternId _id = 0;
	Expression *_root = nullptr;
	FlagMatrix *_flagMatrix = nullptr;
	std::vector<Area> *_searchAreas = nullptr;
	Size _imageSize;
	//Not included in serialization or equality
	Point *_found = nullptr;
	static FlagMatrix *CreateFlagMatrix(const Size &imageSize, const std::vector<Area> &searchAreas);
public:
	static const wchar_t* PIXEL_PATTERN_FILE_EXT;
	PixelPattern(Size imageSize, PatternId id, Expression *root, std::vector<Area> *searchAreas = nullptr);
	~PixelPattern();
	JsonPersistableDef(PixelPattern);
	static PixelPattern *FromFile(const std::string &file);
	inline const Point *Found() const { return _found; }
	inline bool Changed() const { return _changed; }
	inline PatternId Id() const { return _id; }
	void Reset();
	void Update(const Bitmap &ss);
	bool operator==(const PixelPattern &rhs) const;
	bool operator!=(const PixelPattern &rhs) const;
};

typedef std::unordered_map<PatternId, PixelPattern*> PatternMap;
struct Parser {
	explicit Parser(const Size &imageSize);
	virtual ~Parser();
	void AddPattern(const PixelPattern &pattern);
	void RemovePattern(PatternId id);
	const PixelPattern *GetPattern(PatternId id) const;
protected:
	const Size _imageSize;
	PatternMap *_patterns;
	virtual void _Parse(const Bitmap &bmp, bool reset);
};

struct SingleParser : public Parser {
	explicit SingleParser(const Size &imageSize);
	void Parse(const Bitmap &bmp);
};

struct SeriesParser : public Parser {
	explicit SeriesParser(const Size &imageSize);
	void Next(const Bitmap &bmp, bool reset = false);
};

IMGEXP_NS_END

#endif //_IMGEXP_H_