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

#ifndef __imgexp_H__
#define __imgexp_H__

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

namespace imgexp {

//Helpers
std::string GetAllText(const std::string &fileName);

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
    Serialization
};

struct Exception : std::exception {
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
#define ThrowIOWrite(message) __Throw(ErrorCode::IOWrite, message)
#define ThrowArgument(message) __Throw(ErrorCode::Argument, message)
#define ThrowKeyNotFound(key) __Throw(ErrorCode::KeyNotFound, key)
#define ThrowDuplicateKey(key) __Throw(ErrorCode::DuplicateKey, key)
#define ThrowDeserialization(message) __Throw(ErrorCode::Deserialization, message)
#define ThrowSerialization(message) __Throw(ErrorCode::Serialization, message)
#define ThrowFileNotFound(fileName) __Throw(ErrorCode::FileNotFound, fileName)
#define ThrowInvalidType __Throw(ErrorCode::InvalidType, (char*) 0)

#pragma region helpers
void RequireTypeName(const Json::Value &value, const std::string &cmpType);
struct Operand;
Operand *CreateOperand(const Json::Value &value);
//Definitions to make a type persistable to/from Json.
//This should only be applied to concreate types.
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

    inline unsigned long Height() const { return _height; }
    inline unsigned long Width() const { return _width; }
    inline unsigned long Area() const { return _area; }
    inline bool operator==(const Size &rhs) const
    {
        return _height == rhs._height && _width == rhs._width;
    }
    inline bool operator!=(const Size &rhs) const
    {
        return _height != rhs._height || _width != rhs._width;
    }
};
class Point {
    long _x = 0;
    long _y = 0;
public:
    JsonPersistableDef(Point);
    inline long X() const {
        return _x;
    }
    inline long Y() const {
        return _y;
    }
    static const Point Empty;
    Point();
    Point(const Point &rhs);
    Point(long x, long y);
    inline void Set(long x, long y)
    {
        _x = x;
        _y = y;
    }
    inline Point &operator=(const Point &rhs)
    {
        if (&rhs != this)
        {
            _x = rhs._x;
            _y = rhs._y;
        }

        return *this;
    }
    inline Point &operator+=(const Point &rhs)
    {
        _x += rhs._x;
        _y += rhs._y;
        return *this;
    }
    inline bool operator==(const Point &rhs) const
    {
        return _x == rhs._x && _y == rhs._y;
    }
    inline Point operator+(const Point &rhs) const
    {
        return Point(_x + rhs._x, _y + rhs._y);
    }
    inline Point operator-(const Point &rhs) const
    {
        return Point(_x - rhs._x, _y - rhs._y);
    }
    inline bool operator!=(const Point &rhs) const
    {
        return !operator==(rhs);
    }
    inline bool operator< (const Point &rhs) const
    {
        return _y == rhs._y ? _x < rhs._x : _y < rhs._y;
    }
    inline bool operator>(const Point &rhs) const
    {
        return  rhs < *this;
    }
    inline bool operator<=(const Point &rhs) const
    {
        return !operator>(rhs);
    }
    inline bool operator>=(const Point &rhs) const
    {
        return !operator< (rhs);
    }
};
class Area {
    Point _topLeft;
    Point _bottomRight;
public:
    JsonPersistableDef(Area);
    inline Point TopLeft() const {
        return _topLeft;
    }
    inline Point BottomRight() const {
        return _bottomRight;
    }
    inline long Left() const {
        return _topLeft.X();
    }
    inline long Top() const  {
        return _topLeft.Y();
    }
    inline long Right() const {
        return _bottomRight.X();
    }
    inline long Bottom() const {
        return _bottomRight.Y();
    }
    inline long Height() const {
        return Bottom() - Top() + 1;
    }
    inline long Width() const {
        return Right() - Left() + 1;
    }
    Area(long left, long top, long right, long bottom);
    Area(const Point &topLeft, const Point &bottomRight);
    //Gets the area "other" overlaps this Area
    inline Area *GetOverlappedArea(const Area &other) const
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
    inline bool Overlaps(const Area &other) const
    {
        return Contains(other._topLeft) || Contains(other._bottomRight);
    }
    inline bool Contains(const Area &other) const
    {
        return Contains(other._topLeft) && Contains(other._bottomRight);
    }
    inline bool Contains(const Point &pt) const
    {
        return pt.X() >= Left() && pt.X() <= Right() &&
                pt.Y() >= Top() && pt.Y() <= Bottom();
    }
    inline Area &operator=(const Area &rhs)
    {
        if (&rhs != this)
        {
            _topLeft = rhs._topLeft;
            _bottomRight = rhs._bottomRight;
        }

        return *this;
    }
    inline bool operator==(const Area &rhs) const
    {
        return Left() == rhs.Left() && Top() == rhs.Top() &&
                Right() == rhs.Right() && Bottom() == rhs.Bottom();
    }
    inline bool operator!=(const Area &rhs) const
    {
        return !operator==(rhs);
    }
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
    inline BYTE Blue() const {
        return _blue;
    }
    inline BYTE Red() const {
        return _red;
    }
    inline BYTE Green() const {
        return _green;
    }
    inline bool operator==(const Color &rhs) const
    {
        return rhs._blue == _blue && rhs._green == _green && rhs._red == _red;
    }
    inline bool operator!=(const Color &rhs) const
    {
        return !operator==(rhs);
    }
    inline bool operator>(const Color &rhs) const
    {
        return _blue > rhs._blue && _green > rhs._green && _red > rhs._red;
    }
    inline bool operator<(const Color &rhs) const
    {
        return _blue < rhs._blue && _green < rhs._green && _red < rhs._red;
    }
    inline bool operator>=(const Color &rhs) const
    {
        return _blue >= rhs._blue && _green >= rhs._green && _red >= rhs._red;
    }
    inline bool operator<=(const Color &rhs) const
    {
        return _blue <= rhs._blue && _green <= rhs._green && _red <= rhs._red;
    }
};
#include <poppack.h>

class Bitmap {
    const Color *_colors;
    const BITMAPINFOHEADER _bitmapInfo;
    const long _width;
    const long _height;
public:
    inline ::imgexp::Size Size() const  {
        return ::imgexp::Size(_width, _height);
    }
    inline long Width() const {
        return _width;
    }
    inline long Height() const {
        return _height;
    }
    static Bitmap *FromFile(const std::string &fileName);
    Bitmap(const BITMAPINFOHEADER &bitmapInfo, const Color colors[]);
    //note: no need for a cctor because _colors is const.
    virtual ~Bitmap();
    void Save(const std::string &fileName) const;
    inline const Color &getColor(const Point &location) const
    {
        return getColor(location.X(), location.Y());
    }
    inline const Color &getColor(const long x, const long y) const
    {
        return _colors[(_height - 1 - y)*_width + x];
    }
};
typedef std::shared_ptr<Bitmap> BitmapPtr;
#pragma endregion

#pragma region expression tree
class PixelPattern;

struct Operand {
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
protected:
    Point _offset;
    PixelMatch();
    virtual bool Equals(const Operand &rhs) const
    {
        if (auto p = dynamic_cast<PixelMatch const*>(&rhs))
        {
            return _offset == p->_offset;
        }
        else
            return false;
    }
public:
    VJsonPersistableDef(PixelMatch) = 0;
    virtual bool Eval(const Bitmap &ss, const Point &start) const = 0;
    inline void setOffset(const Point &value)
    {
        _offset = value;
    }
    inline const Point &Offset() {
        return _offset;
    }
};

//A pixel color matches exactly
typedef ::imgexp::Color _Color;
class ExactPixelMatch : public PixelMatch {
    _Color _color;
protected:
    virtual bool Equals(const Operand &rhs) const
    {
        if (auto p = dynamic_cast<ExactPixelMatch const*>(&rhs))
        {
            return PixelMatch::Equals(rhs) && _color == p->_color;
        }
        else
            return false;
    }
public:
    VJsonPersistableDef(ExactPixelMatch);
    ExactPixelMatch(const Color &color);
    virtual bool Eval(const Bitmap &ss, const Point &start) const;

    inline const _Color &Color()
    {
        return _color;
    }
};

//A pixel color matches within a range of colors
class RangePixelMatch : public PixelMatch {
    Color _min;
    Color _max;
protected:
    virtual bool Equals(const Operand &rhs) const
    {
        if (auto p = dynamic_cast<RangePixelMatch const*>(&rhs))
        {
            return PixelMatch::Equals(rhs) && _min == p->_min && _max == p->_max;
        }
        else
            return false;
    }
public:
    VJsonPersistableDef(RangePixelMatch);
    RangePixelMatch(const Color &min, const Color &max);
    virtual bool Eval(const Bitmap &ss, const Point &start) const;
    inline const Color &Min() {
        return _min;
    }
    inline const Color &Max() {
        return _max;
    }
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

inline Operator StringToOperator(const std::string &str)
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
inline std::string OperatorToString(Operator value)
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

class Expression : public Operand {
    Operand * _left = nullptr;
    ::imgexp::Operator _operator = ::imgexp::Operator::NONE;
    Operand * _right = nullptr;
public:
    virtual ~Expression();
    inline Operand &Left() {
        return *_left;
    }
    inline ::imgexp::Operator Operator() {
        return _operator;
    }
    inline void setOperator(::imgexp::Operator value) {
        _operator = value;
    }
    inline Operand *Right() {
        return _right;
    }
    inline void setRight(Operand *value) {
        _right = value;
    }
    VJsonPersistableDef(Expression);
    Expression(Operand *left, ::imgexp::Operator op = ::imgexp::Operator::NONE, Operand *right = nullptr);
    virtual bool Eval(const Bitmap &ss, const Point &start) const;
protected:
    virtual bool Equals(const Operand &rhs) const
    {
        if (auto p = dynamic_cast<Expression const*>(&rhs))
        {
            bool left = _left && p->_left && *_left == *p->_left;
            bool right = _right && p->_right && *_right == *p->_right;
            bool op = _operator == p->_operator;
            return left && right && op;
        }
        else
            return false;
    }
};

typedef std::vector<std::vector<bool>> FlagMatrix;
typedef unsigned long PatternId;
class PixelPattern {
    bool _changed;
    PatternId _id;
    Expression *_root;
    FlagMatrix *_flagMatrix = nullptr;
    std::vector<Area> *_searchAreas = nullptr;
    Size _imageSize;
    //Not included in serialization or equality
    Point *_found = nullptr;
    static FlagMatrix *CreateFlagMatrix(const Size &imageSize, const std::vector<Area> &searchAreas);
public:
    static const wchar_t* PIXEL_PATTERN_FILE_EXT;
    PixelPattern(Size imageSize, PatternId id, Expression &root, std::vector<Area> *searchAreas = nullptr);
    ~PixelPattern();
    JsonPersistableDef(PixelPattern);
    static PixelPattern *FromFile(const std::string &file);
    inline const Point *Found() const { return _found; }
    inline bool Changed() const { return _changed; }
    inline PatternId Id() const { return _id; }
    void Reset();
    void Update(const Bitmap &ss);
    bool operator==(const PixelPattern &rhs) const
    {
        return _id == rhs._id;
    }
    bool operator!=(const PixelPattern &rhs) const
    {
        return !operator==(rhs);
    }
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

}

#endif //__imgexp_H__
