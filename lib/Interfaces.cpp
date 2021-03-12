#include "Interfaces.h"

#include <sstream>

namespace descartes {

namespace {

std::string tokenKindToString(TokenKind kind) {
  switch (kind) {
  case TokenKind::Identifier:
    return "Identifier";
  case TokenKind::Number:
    return "Number";
  case TokenKind::String:
    return "String";
  case TokenKind::Add:
    return "Add";
  case TokenKind::Subtract:
    return "Subtract";
  case TokenKind::Multiply:
    return "Multiply";
  case TokenKind::Divide:
    return "Divide";
  case TokenKind::Equal:
    return "Equal";
  case TokenKind::LessThan:
    return "LessThan";
  case TokenKind::GreaterThan:
    return "GreaterThan";
  case TokenKind::OpenBracket:
    return "OpenBracket";
  case TokenKind::CloseBracket:
    return "CloseBracket";
  case TokenKind::Period:
    return "Period";
  case TokenKind::Comma:
    return "Comma";
  case TokenKind::Colon:
    return "Colon";
  case TokenKind::SemiColon:
    return "SemiColon";
  case TokenKind::Hat:
    return "Hat";
  case TokenKind::OpenParen:
    return "OpenParen";
  case TokenKind::CloseParen:
    return "CloseParen";
  case TokenKind::NotEqual:
    return "NotEqual";
  case TokenKind::LessThanEqual:
    return "LessThanEqual";
  case TokenKind::GreaterThanEqual:
    return "GreaterThanEqual";
  case TokenKind::Assign:
    return "Assign";
  case TokenKind::DoublePeriod:
    return "DoublePeriod";
  case TokenKind::And:
    return "And";
  case TokenKind::Array:
    return "Array";
  case TokenKind::Begin:
    return "Begin";
  case TokenKind::Case:
    return "Case";
  case TokenKind::Const:
    return "Const";
  case TokenKind::Div:
    return "Div";
  case TokenKind::Do:
    return "Do";
  case TokenKind::DownTo:
    return "DownTo";
  case TokenKind::Else:
    return "Else";
  case TokenKind::End:
    return "End";
  case TokenKind::File:
    return "File";
  case TokenKind::For:
    return "For";
  case TokenKind::Function:
    return "Function";
  case TokenKind::GoTo:
    return "GoTo";
  case TokenKind::If:
    return "If";
  case TokenKind::In:
    return "In";
  case TokenKind::Label:
    return "Label";
  case TokenKind::Mod:
    return "Mod";
  case TokenKind::Nil:
    return "Nil";
  case TokenKind::Not:
    return "Not";
  case TokenKind::Of:
    return "Of";
  case TokenKind::Or:
    return "Or";
  case TokenKind::Packed:
    return "Packed";
  case TokenKind::Procedure:
    return "Procedure";
  case TokenKind::Program:
    return "Program";
  case TokenKind::Record:
    return "Record";
  case TokenKind::Repeat:
    return "Repeat";
  case TokenKind::Set:
    return "Set";
  case TokenKind::Then:
    return "Then";
  case TokenKind::To:
    return "To";
  case TokenKind::Type:
    return "Type";
  case TokenKind::Until:
    return "Until";
  case TokenKind::Var:
    return "Var";
  case TokenKind::While:
    return "While";
  case TokenKind::With:
    return "With";
  case TokenKind::Eof:
    return "Eof";
  }
}

} // namespace

std::string Token::toString() const {
  std::stringstream ss;
  ss << "Kind: " << tokenKindToString(kind) << "\n";
  ss << "Value: " << (val.empty() ? "NONE" : val) << "\n";
  return ss.str();
}

Token::operator bool() const { return kind != TokenKind::Eof; }

bool Token::operator==(const Token &other) const {
  return kind == other.kind && val == other.val;
}

LexerError::operator std::string() const { return std::runtime_error::what(); }

ParserError::operator std::string() const { return std::runtime_error::what(); }

} // namespace descartes
