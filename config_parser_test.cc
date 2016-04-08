#include "gtest/gtest.h"
#include "config_parser.h"
#include <sstream>
#include <string>

class NginxStringConfigTest : public ::testing::Test {
 protected:
  bool ParseString(const std::string& config_string) {
    std::stringstream config_stream(config_string);
    return parser_.Parse(&config_stream, &out_config_);
  }
  NginxConfigParser parser_;
  NginxConfig out_config_;
};

// Test simple statement
TEST_F(NginxStringConfigTest, SimpleConfig) {
  EXPECT_TRUE(ParseString("foo bar;"));
  // We can also add expectations about the contents of the statements:
  EXPECT_EQ(1, out_config_.statements_.size());
  EXPECT_EQ("foo", out_config_.statements_.at(0)->tokens_.at(0));
}

// Test config with 2 statements
TEST_F(NginxStringConfigTest, TwoLineConfig) {
  EXPECT_TRUE(ParseString("foo bar;\nfizz buzz;"));

  EXPECT_EQ(2, out_config_.statements_.size());
  EXPECT_EQ("foo", out_config_.statements_.at(0)->tokens_.at(0));
  EXPECT_EQ("fizz", out_config_.statements_.at(1)->tokens_.at(0));
}

// Test config with a comment
TEST_F(NginxStringConfigTest, CommentedConfig) {
  EXPECT_TRUE(ParseString("foo bar;\n# This should be ignored\nfizz buzz;"));

  // Result should be same as TwoLineConfig
  EXPECT_EQ(2, out_config_.statements_.size());
  EXPECT_EQ("foo", out_config_.statements_.at(0)->tokens_.at(0));
  EXPECT_EQ("fizz", out_config_.statements_.at(1)->tokens_.at(0));
}

// Test a block config statement
TEST_F(NginxStringConfigTest, BlockConfig) {
  EXPECT_TRUE(ParseString("server {\n  listen 80;\n  root html;\n}"));
  EXPECT_EQ(1, out_config_.statements_.size());
  EXPECT_EQ("server", out_config_.statements_.at(0)->tokens_.at(0));

  // Make sure nested block is correct
  NginxConfig child_config_ = *out_config_.statements_.at(0)->child_block_;
  EXPECT_EQ("listen", child_config_.statements_.at(0)->tokens_.at(0));
}

// Test an incorrect config
TEST_F(NginxStringConfigTest, WrongConfig) {
  EXPECT_FALSE(ParseString("foo bar"));
}

// Test nested blocks
TEST_F(NginxStringConfigTest, NestedConfig) {
  EXPECT_TRUE(ParseString("server {\n  location / {\n    foo bar;\n  }\n}"));
  EXPECT_EQ(1, out_config_.statements_.size());
  EXPECT_EQ("server", out_config_.statements_.at(0)->tokens_.at(0));

  // Make sure nested blocks are correct
  NginxConfig child_config_ = *out_config_.statements_.at(0)->child_block_;
  EXPECT_EQ("location", child_config_.statements_.at(0)->tokens_.at(0));
  child_config_ = *child_config_.statements_.at(0)->child_block_;
  EXPECT_EQ("foo", child_config_.statements_.at(0)->tokens_.at(0));
}

// Test nested blocks w/o two curly braces in a row
TEST_F(NginxStringConfigTest, StatementAfterNestedConfig) {
  EXPECT_TRUE(ParseString("server {\n  location / {\n    foo bar;\n  }\n    fizz buzz;\n}"));

  // Make sure nested block is correct
  NginxConfig child_config_ = *out_config_.statements_.at(0)->child_block_;
  EXPECT_EQ(2, child_config_.statements_.size());
  EXPECT_EQ("location", child_config_.statements_.at(0)->tokens_.at(0));
  EXPECT_EQ("fizz", child_config_.statements_.at(1)->tokens_.at(0));
}

// Test parsing code from a file
TEST(NginxConfigParserTest, ConfigFile) {
  NginxConfigParser parser;
  NginxConfig out_config;

  bool success = parser.Parse("example_config", &out_config);

  EXPECT_TRUE(success);
}
