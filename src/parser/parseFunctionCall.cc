

#include "../common.h"
#include "../lexer/tokens/token.h"
#include "./Parser.h"

#include <assert.h>

using namespace snowball::Syntax::Expression;

namespace snowball::parser {

FunctionCall*
Parser::parseFunctionCall(Syntax::Expression::Base* callee, TokenType terminator, std::string terminatorString) {
  assert(callee != nullptr);
  std::vector<Base*> arguments;
  while (true) {
    auto pk = peek();
    if (terminator == pk.type) { break; }
    auto val = parseExpr(false);
    arguments.push_back(val);
    pk = peek();
    if (is<TokenType::SYM_COMMA>(pk) || terminator == pk.type) {
      if (is<TokenType::SYM_COMMA>(pk)) next();
      continue;
    } else {
      next();
      createError<SYNTAX_ERROR>(
      FMT("Expected a ',' or a '%s' but found '%s' instead", terminatorString.c_str(), pk.to_string().c_str())
      );
    }
  }
  next(); // Consume "("
  auto call = Syntax::N<FunctionCall>(callee, arguments);
  //dbg->width += 2;
  // TODO: dbg info should start at the "(", maybe?
  call->setSourceInfo(m_source_info);
  call->setDBGInfo(callee->getDBGInfo());
  return call;
}

} // namespace snowball::parser
