#include <gtest/gtest.h>
#include "json/json.h"
#include "imgexp.h"
#include "imgexputil.h"
#include <boost/filesystem.hpp>
#include <map>

using namespace std;
using namespace imgexp;
using namespace imgexp::util;

string ProjectDir("..\\..\\test\\");
string ImagesDir(ProjectDir + "testdata\\images\\");
string ColorsImagesDir(ImagesDir + "colors\\");
string FindImagesDir(ImagesDir + "find\\");

namespace imgexptest {
	void VerifyAllColor(const Bitmap &bmp, const Color &c)
	{
		auto xEnd = bmp.Width();
		auto yEnd = bmp.Height();

		for (long y = 0; y < yEnd; ++y)
		{
			for (long x = 0; x < xEnd; ++x)
				EXPECT_TRUE(bmp.Color(x, y) == c);
		}
	}
	void VerifyLoadFromFileAndVerifyColors(std::string fileName, Color &color)
	{
		auto image = Bitmap::FromFile(ColorsImagesDir + fileName);
		VerifyAllColor(*image, color);
		delete image;
	}
	std::string WriteJsonToTempFile(const Json::Value &value)
	{
		auto tempDir = boost::filesystem::temp_directory_path();
		auto tempFile = boost::filesystem::path(tempDir) / boost::filesystem::path(boost::filesystem::unique_path());

		Json::StyledWriter writer;
		string jsonStr = writer.write(value);
		ofstream output(tempFile.c_str(), ios_base::trunc);
		output << jsonStr;
		output.close();
		return tempFile.generic_string();
	}
	class BitmapTests : public ::testing::Test {
	};
	class ExpressionTests : public ::testing::Test {
	};
	class PixelPatternTests : public ::testing::Test {
	};
	//=========================================================================
	//== BitmapTests
	//=========================================================================
	TEST_F(BitmapTests, LoadsFromFile)
	{
		auto image = Bitmap::FromFile(ColorsImagesDir + "red.bmp");
		EXPECT_NE(nullptr, image);

		delete image;
	}
	TEST_F(BitmapTests, AllPrimaryColorFilesLoadAndAreTheirExpectedColors)
	{
		VerifyLoadFromFileAndVerifyColors("red.bmp", Color(0xff, 0, 0));
		VerifyLoadFromFileAndVerifyColors("green.bmp", Color(0, 0xff, 0));
		VerifyLoadFromFileAndVerifyColors("blue.bmp", Color(0, 0, 0xff));
		VerifyLoadFromFileAndVerifyColors("black.bmp", Color(0, 0, 0));
		VerifyLoadFromFileAndVerifyColors("white.bmp", Color(0xff, 0xff, 0xff));
	}
	//=========================================================================
	//== ExpressionTests
	//=========================================================================
	TEST_F(ExpressionTests, CannotBeCreatedWithEmptyLeftAndRight)
	{
		EXPECT_THROW(new Expression(nullptr, imgexp::Operator::NONE, nullptr), Exception);
	}
	TEST_F(ExpressionTests, WithLeftAndRightSavesAndReloadsTheSame)
	{
		auto right = new ExactPixelMatch(Color(0xff, 0, 0));
		right->Offset(Point(0, 0));
		auto left = new ExactPixelMatch(Color(0xff, 0, 0));
		left->Offset(Point(0, 1));
		auto exp = new Expression(left, imgexp::Operator::AND, right);
		auto tempFile = WriteJsonToTempFile((Json::Value)*exp);
		auto node = ParseJsonFromFile(tempFile);
		DeleteFile(tempFile.c_str());
		auto newExp = new Expression(node);
		EXPECT_EQ(*exp, *newExp);
		delete exp;
	}
	TEST_F(ExpressionTests, WithEmptyRightSavesAndReloadsTheSame)
	{
		auto left = new ExactPixelMatch(Color(0xff, 0, 0));
		left->Offset(Point(0, 0));
		auto exp = new Expression(left);
		auto tempFile = WriteJsonToTempFile((Json::Value)*exp);
		auto node = ParseJsonFromFile(tempFile);
		DeleteFile(tempFile.c_str());
		auto newExp = new Expression(node);
		EXPECT_EQ(*exp, *newExp);
		delete exp;
	}
	//=========================================================================
	//== PixelPatternTests
	//=========================================================================
	TEST_F(PixelPatternTests, SavesAndReloadsTheSame)
	{
		PixelPattern pp(Size(5, 5), 1, new Expression(new ExactPixelMatch(Color(0xff, 0, 0))));

		auto tempFile = WriteJsonToTempFile(pp);
		auto node = ParseJsonFromFile(tempFile);
		DeleteFile(tempFile.c_str());

		PixelPattern newPp(node);

		EXPECT_EQ(pp, newPp);
	}
	TEST_F(PixelPatternTests, ExactPixelMatchFindsAllBlips)
	{
		Color c(0, 0xff, 0xff);

		//build the pattern
		std::map<Point, PixelMatch*> pointMatches = {
			{ Point(198, 24), new ExactPixelMatch(c) },
			{ Point(204, 29), new ExactPixelMatch(c) },
			{ Point(196, 31), new ExactPixelMatch(c) },
			{ Point(204, 33), new ExactPixelMatch(c) },
			{ Point(197, 39), new ExactPixelMatch(c) },
			{ Point(206, 41), new ExactPixelMatch(c) },
		};

		auto root = BuildExpressionTree(pointMatches);

		PixelPattern pattern(Size(1024, 768), 1, root);
		auto image = Bitmap::FromFile(FindImagesDir + "0255255blips.bmp");

		pattern.Update(*image);

		EXPECT_EQ(Point(198, 24), *pattern.Found());
	}
	TEST_F(PixelPatternTests, RangePixelMatchDoesNotFindAllBlipsDueToOffPoint)
	{
		Color mn(1, 1, 1);
		Color mx(150, 150, 150);

		std::map<Point, PixelMatch*> pointMatches = {
			{ Point(198, 24), new RangePixelMatch(mn, mx) },
			{ Point(204, 29), new RangePixelMatch(mn, mx) },
			{ Point(196, 31), new RangePixelMatch(mn, mx) },
			{ Point(204, 33), new RangePixelMatch(mn, mx) },
			{ Point(197, 39), new RangePixelMatch(mn, mx) },
			{ Point(206, 40), new RangePixelMatch(mn, mx) }, //off by one
		};

		auto root = BuildExpressionTree(pointMatches);

		PixelPattern pattern(Size(1024, 768), 1, root);
		auto image = Bitmap::FromFile(FindImagesDir + "111-150150150blips.bmp");

		pattern.Update(*image);

		EXPECT_EQ(nullptr, pattern.Found());
	}
	TEST_F(PixelPatternTests, RangePixelMatchFindsAllBlips)
	{
		Color mn(1, 1, 1);
		Color mx(150, 150, 150);

		std::map<Point, PixelMatch*> pointMatches = {
			{ Point(198, 24), new RangePixelMatch(mn, mx) },
			{ Point(204, 29), new RangePixelMatch(mn, mx) },
			{ Point(196, 31), new RangePixelMatch(mn, mx) },
			{ Point(204, 33), new RangePixelMatch(mn, mx) },
			{ Point(197, 39), new RangePixelMatch(mn, mx) },
			{ Point(206, 41), new RangePixelMatch(mn, mx) },
		};

		auto root = BuildExpressionTree(pointMatches);

		PixelPattern pattern(Size(1024, 768), 1, root);
		auto image = Bitmap::FromFile(FindImagesDir + "111-150150150blips.bmp");

		pattern.Update(*image);

		EXPECT_EQ(Point(198, 24), *pattern.Found());
	}
	TEST_F(PixelPatternTests, RangePixelMatchDoesNotFindAllBlipsDueToOffColor)
	{
		Color mn(1, 1, 1);
		Color mx(150, 150, 150);

		std::map<Point, PixelMatch*> pointMatches = {
			{ Point(198, 24), new RangePixelMatch(mn, mx) },
			{ Point(204, 29), new RangePixelMatch(mn, mx) },
			{ Point(196, 31), new RangePixelMatch(mn, mx) },
			{ Point(204, 33), new RangePixelMatch(mn, mx) },
			{ Point(197, 39), new RangePixelMatch(mn, mx) },
			{ Point(206, 41), new RangePixelMatch(mn, mx) },
		};

		auto root = BuildExpressionTree(pointMatches);

		PixelPattern pattern(Size(1024, 768), 1, root);
		auto image = Bitmap::FromFile(FindImagesDir + "111-150150150blips_oneoffcolor.bmp");

		pattern.Update(*image);

		EXPECT_EQ(nullptr, pattern.Found());
	}
}

