
#include "DocGen.h"
#include "DocTemplates.h"
#include "../../services/OperatorService.h"
#include <optional>

namespace snowball {
namespace Syntax {
namespace docgen {

static std::unordered_map<std::string, int> nameCount;
#define GO_BACK_TEXT "&lt; Go Back"

#define GENERATE_TOP_LEVEL_NODES(nodes) \
  auto functions = std::vector<Statement::FunctionDef*>(); \
  auto types = std::vector<Statement::Base*>(); \
  auto namespaces = std::vector<Statement::Namespace*>(); \
  auto macros = std::vector<Macro*>(); \
  auto variables = std::vector<Statement::VariableDecl*>();\
  for (auto& node : nodes) {\
    if (auto func = utils::cast<Statement::FunctionDef>(node)) { \
      if (func->isPublic())\
        functions.push_back(func); \
    } else if (auto type = utils::cast<Statement::DefinedTypeDef>(node)) { \
      if (type->isPublic()) \
        types.push_back(type); \
    } else if (auto ns = utils::cast<Statement::Namespace>(node)) { \
      namespaces.push_back(ns); \
    } else if (auto macro = utils::cast<Macro>(node)) { \
      if (macro->hasAttribute(Attributes::EXPORT)) \
        macros.push_back(macro); \
    } else if (auto var = utils::cast<Statement::VariableDecl>(node)) { \
      if (var->isPublic()) \
        variables.push_back(var); \
    } else if (auto typeAlias = utils::cast<Statement::TypeAlias>(node)) { \
      if (typeAlias->isPublic()) \
        types.push_back(typeAlias); \
    } \
  } \
  if (types.size() > 0) { \
    body += "<br/><h1 style=\"font-size: 25px;\">Types exported from " + utils::join(nameParts.begin(), nameParts.end(), "::") + "</h1><br/>"; \
    for (auto& type : types) { \
      bool isAlias = utils::is<Statement::TypeAlias>(type); \
      auto typeUrl = page.path.string().substr(0, page.path.string().size() - 5) + "/" + (isAlias ? utils::cast<Statement::TypeAlias>(type)->getIdentifier() : utils::cast<Statement::DefinedTypeDef>(type)->getName()) + ".html"; \
      body += "<div style=\"display: grid; grid-template-columns: 1fr 1fr 1fr;\"><a href=" + typeUrl + "><h1 style=\"color:rgb(45 212 191);margin-right: 10px;font-weight: normal;\">" + (isAlias ? utils::cast<Statement::TypeAlias>(type)->getIdentifier() : utils::cast<Statement::DefinedTypeDef>(type)->getName()); \
      if (isAlias) body += "<span class=\"tag\">alias</span>"; \
      body += "</h1></a>"; \
      auto comment = isAlias ? utils::cast<Statement::TypeAlias>(type)->getComment() : utils::cast<Statement::DefinedTypeDef>(type)->getComment(); \
      if (comment) { \
        auto values = comment->getValues(); \
        if (values.count("brief")) { \
          auto brief = sanitize(values["brief"]); \
          utils::replaceAll(brief, "<br/>", " "); \
          body += "<span>" + brief + "</span>"; \
        } \
      } \
      body += "</div>"; \
    } \
  } \
  if (namespaces.size() > 0) { \
    body += "<br/><h1 style=\"font-size: 25px;\">Namespaces exported from " + utils::join(nameParts.begin(), nameParts.end(), "::") + "</h1><br/>"; \
    for (auto& ns : namespaces) { \
      auto nsUrl = page.path.string().substr(0, page.path.string().size() - 5) + "/" + ns->getName() + ".html";\
      body += "<div style=\"display: grid; grid-template-columns: 1fr 1fr 1fr;\"><a href=" + nsUrl + "><h1 style=\"color:#d2991d;margin-right: 10px;font-weight: normal;\">" + ns->getName() + "</h1></a>";\
      if (ns->getComment()) { \
        auto values = ns->getComment()->getValues(); \
        if (values.count("brief")) { \
          auto brief = sanitize(values["brief"]); \
          utils::replaceAll(brief, "<br/>", " "); \
          body += "<span>" + brief + "</span>"; \
        } \
      } \
      body += "</div>"; \
    } \
  } \
  if (macros.size() > 0) { \
    body += "<br/><h1 style=\"font-size: 25px;\">Macros exported from " + utils::join(nameParts.begin(), nameParts.end(), "::") + "</h1><br/>"; \
    for (auto& macro : macros) { \
      auto macroUrl = page.path.string().substr(0, page.path.string().size() - 5) + "/" + macro->getName() + "-macro.html"; \
      body += "<div style=\"display: grid; grid-template-columns: 1fr 1fr 1fr;\"><a href=" + macroUrl + "><h1 style=\"color:#2bab63;margin-right: 10px;font-weight: normal;\">" + macro->getName() + "</h1></a>"; \
      if (macro->getComment()) { \
        auto values = macro->getComment()->getValues(); \
        if (values.count("brief")) { \
          auto brief = sanitize(values["brief"]); \
          utils::replaceAll(brief, "<br/>", " "); \
          body += "<span>" + brief + "</span>"; \
        } \
      } \
      body += "</div>"; \
    } \
  } \
  if (variables.size() > 0) { \
    body += "<br/><h1 style=\"font-size: 25px;\">Variables exported from " + utils::join(nameParts.begin(), nameParts.end(), "::") + "</h1><br/>"; \
    for (auto& var : variables) { \
      auto varUrl = page.path.string().substr(0, page.path.string().size() - 5) + "/" + var->getName() + "-var.html"; \
      body += "<div style=\"display: grid; grid-template-columns: 1fr 1fr 1fr;\"><a href=" + varUrl + "><h1 style=\"color:rgb(167 139 250);margin-right: 10px;font-weight: normal;\">" + var->getName(); \
      if (var->isContantDecl()) body += "<span class=\"tag\">const</span>"; \
      body += "</h1></a>"; \
      if (var->getComment()) { \
        auto values = var->getComment()->getValues(); \
        if (values.count("brief")) { \
          auto brief = sanitize(values["brief"]); \
          utils::replaceAll(brief, "<br/>", " "); \
          body += "<span>" + brief + "</span>"; \
        } \
      } \
      body += "</div>"; \
    } \
  } \
  if (functions.size() > 0) {\
    body += "<br/><h1 style=\"font-size: 25px;\">Functions exported from " + utils::join(nameParts.begin(), nameParts.end(), "::") + "</h1><br/>"; \
    for (auto& func : functions) { \
      auto funcUrl = page.path.string().substr(0, page.path.string().size() - 5) + "/" + func->getName(); \
      nameCount[funcUrl]++; \
      if (nameCount[funcUrl] > 1) { \
        funcUrl += "-" + std::to_string(nameCount[funcUrl]); \
      } \
      funcUrl += ".html"; \
      body += "<div style=\"display: grid; grid-template-columns: 1fr 1fr 1fr;\"><a href=" + funcUrl + "><h1 style=\"color:rgb(14 116 144);margin-right: 10px;font-weight: normal;\">" + func->getName() + "</h1></a>"; \
      if (func->getComment()) { \
        auto values = func->getComment()->getValues(); \
        if (values.count("brief")) { \
          auto brief = sanitize(values["brief"]); \
          utils::replaceAll(brief, "<br/>", " "); \
          body += "<span>" + brief + "</span>"; \
        } \
      } \
      body += "</div>"; \
    } \
    nameCount.clear(); \
  }

std::string getPageTemplate(DocGenContext context, std::string title, std::string body,
                            std::vector<std::pair<std::string, std::string>> links) {
  std::string parent;
  std::string parentPath;
  if (!context.currentType.empty()) {
    parent = context.currentType;
    parentPath = context.currentTypePath;
  } else if (!context.currentNamespace.empty()) {
    parent = context.currentNamespace;
    parentPath = context.currentNamespacePath;
  } else {
    parent = context.currentModule;
    parentPath = context.currentModulePath;
  }
  std::sort(links.begin(), links.end(), [](auto & a, auto & b) {
              return a.first < b.first;
            });
  links.insert(links.begin(), {GO_BACK_TEXT, parentPath});
  std::string linksHTML = "";
  for (auto& [name, link] : links) {
      nameCount[name]++;
      auto resultLink = link;
      if (nameCount[name] > 1) {
        resultLink += "-" + std::to_string(nameCount[name]);
      }
      auto resultName = nameCount[name] > 1 ? name + "(" + std::to_string(nameCount[name]) + ")" : name;
      if (resultName != GO_BACK_TEXT)
        resultName = "&gt; " + resultName;
        linksHTML += "<a href=\"" + resultLink + ".html\">" + resultName + "</a> ";
      }
      nameCount.clear();
  return FMT(pageTemplate.c_str(), title.c_str(), context.baseURL.c_str(), context.currentModule.c_str(),
             context.packageVersion.c_str(), linksHTML.c_str(), body.c_str());
}

namespace {
std::string generateFromBase(Expression::Base* ast) {
  if (auto x = utils::cast<Expression::Identifier>(ast)) {
    auto n = x->getIdentifier();
    if (auto generics = utils::cast<Expression::GenericIdentifier>(x)) {
      n += "&lt;";
      for (auto& g : generics->getGenerics()) {
        n += typeToHtml(g) + ", ";
      }
      if (generics->getGenerics().size() > 0)
        n = n.substr(0, n.size() - 2);
      n += "&gt;";
    }
    return n;
  } else if (auto x = utils::cast<Expression::Index>(ast)) {
    auto base = x->getBase();
    auto identifier = x->getIdentifier();
    return generateFromBase(base) + "::" + generateFromBase(identifier);
  }
  assert(false);
}

std::string sanitize(std::string s) {
  auto str = s;
  utils::replaceAll(str, "<", "&lt;");
  utils::replaceAll(str, ">", "&gt;");
  utils::replaceAll(str, "\n", "<br/>");
  auto trimed = s;
  utils::replaceAll(trimed, " ", "");
  if (trimed == "<br/>") return "";
  while (utils::endsWith(str, "<br/>"))
    str = str.substr(0, str.size() - 5);
  return "<br/>" + str;
}

std::string getFunctionCode(Statement::FunctionDef* node) {
  std::string body;
  if (node->isExtern()) body += "external ";
  if (node->isMutable()) body += "mut ";
  if (node->isPublic()) body += "public ";
  if (node->isPrivate()) body += "private ";
  if (node->isVirtual()) body += "virtual ";
  if (node->isStatic()) body += "static ";
  if (node->hasAttribute(Attributes::UNSAFE)) body += "unsafe ";
  bool isOperator = Operator::isOperator(node->getName());
  if (isOperator) {
    body += "operator func ";
    auto name = Operator::operatorName(Operator::operatorID(node->getName()));
    if (utils::startsWith(name, "operator ")) {
      body += name.substr(9);
    } else {
      body += name;
    }
  } else {
    body += "func " + node->getName();
  }
  if (node->isGeneric()) {
    body += "&lt;";
    for (auto& generic : node->getGenerics()) {
      body += generic->getName();
      if (generic->getType()) {
        body += " = " + typeToHtml(generic->getType());
      }
      body += ", ";
    }
    // remove the last ", "
    if (node->getGenerics().size() > 0)
      body = body.substr(0, body.size() - 2);
    body += "&gt;";
  }
  body += "(\n";
  for (auto& param : node->getArgs()) {
    body += "    " + param->getName() + ": " + typeToHtml(param->getType());
    if (param->hasDefaultValue()) {
      body += " = ...";
    }
    body += ",\n";
  }
  if (node->isVariadic()) {
    body += "    ...,\n";
  }
  // remove the last ",<br>"
  if (utils::endsWith(body, ",\n"))
    body = body.substr(0, body.size() - 2);
  if (node->isVariadic() || node->getArgs().size() > 0)
    body += "\n";
  else body = body.substr(0, body.size() - 1);
  body += ") " + typeToHtml(node->getRetType()) + " { ... }";
  return body;
}

void createSmallPictureFn(Statement::FunctionDef* node, DocGenContext context, std::string& body) {
  body += "<pre><code style=\"background: transparent !important; padding: 5px 0 !important;\" class=\"language-snowball\">";
  body += getFunctionCode(node);
  body += "</code></pre>";
  if (auto comment = node->getComment()) {
    auto values = comment->getValues();
    if (values.count("brief")) {
      body += "<p style=\"margin-left: 15px; margin-bottom: 20px; margin-top: 10px;\">" + sanitize(
                values["brief"]) + "</p><hr style=\"margin-bottom: 30px;\"/>";
    }
  }
}
}

std::string typeToHtml(Expression::TypeRef* type) {
  if (type->isReferenceType()) {
    return (type->isMutable() ? "&mut " : "&") + typeToHtml(utils::cast<Expression::ReferenceType>(type)->getBaseType());
  } else if (type->isPointerType()) {
    return (type->isMutable() ? "*mut " : "*const ") + typeToHtml(utils::cast<Expression::PointerType>
           (type)->getBaseType());
  } else if (type->isFunctionType()) {
    auto funcType = utils::cast<Expression::FuncType>(type);
    std::string result = "func (";
    for (auto& param : funcType->getArgs()) {
      result += typeToHtml(param) + ", ";
    }
    // remove the last ", "
    result = result.substr(0, result.size() - 2);
    result += ") -> " + typeToHtml(funcType->getReturnValue());
    return result;
  } else if (type->isTypeDecl()) {
    return "decltype(...)";
  }
  auto ast = type->_getInternalAST();
  return ast ? generateFromBase(ast) : type->getName();
}

void createFunctionPage(Statement::FunctionDef* node, DocGenContext context, DocumentationPage& page) {
  std::string body = "";
  body += "<h1>";
  auto pathParts = page.path;
  auto nameParts = utils::list2vec(utils::split(page.name, "::"));
  int i = 0;
  for (auto _ : pathParts) {
    std::string url = "";
    int j = 0;
    for (auto part : pathParts) {
      if (j == i) {
        url += part.string();
        if (utils::endsWith(part.string(), ".html"))
          url = url.substr(0, url.size() - 5);
        break;
      } else {
        url += part.string() + "/";
      }
      j++;
    }
    auto theUrl = nameParts[i];
    if (utils::startsWith(theUrl, "-")) {
      theUrl = theUrl.substr(1); // remove operator prefix
    }
    body += "<a style=\"color:rgb(14 116 144);\" href=\"" + url + ".html\">" + theUrl + "</a>::";
    i++;
  }
  body = body.substr(0, body.size() - 2) + ";"; // remove the last "::"
  body += "</h1><hr/><div><pre><code class=\"language-snowball\">";
  body += getFunctionCode(node);
  body += "</code></pre></div><br><div class=\"doc\">";
  if (auto docString = node->getComment()) {
    auto values = docString->getValues();
    if (values.count("brief")) {
      body += "<h1 style=\"color:rgb(14 116 144);\">Brief Description for <quote>" + node->getName() + "</quote></h1>";
      body += "<p>" + sanitize(values["brief"]) + "</p>";
    }
    auto fnBody = sanitize(docString->getBody());
    if (!fnBody.empty())
      body += "<p>" + fnBody + "</p>";
    std::vector<std::pair<std::string, std::string>> params;
    for (auto& [tag, value] : values) {
        if (utils::startsWith(tag, "param$")) {
          params.push_back({tag, value});
        }
      }
      if (params.size() > 0) {
          body += "<hr/><h1 style=\"color:rgb(14 116 144);\">Parameters</h1>";
          for (auto& [tag, value] : params) {
              std::string paramName = "";
              if (utils::startsWith(value, "[in]")) {
                value = value.substr(4);
            } else if (utils::startsWith(value, "[out]")) {
              value = value.substr(5);
              } else if (utils::startsWith(value, "[in,out]")) {
              value = value.substr(8);
              }
          auto words = utils::list2vec(utils::split(value, " "));
          if (words.size() > 0) {
          for (auto& word : words) {
              if (word.empty()) continue;
              paramName = word;
              break;
            }
            value = value.substr(paramName.size() + 1);
          }
          body += "<h1>" + paramName + "</h1>";
                  body += "<p>" + value + "</p><br/>";
          }
          body = body.substr(0, body.size() - 5);
        }
    for (auto& [tag, value] : values) {
        if (tag == "brief") continue;
        if (utils::startsWith(tag, "param$")) continue;
          body += "<hr/><h1 style=\"color:rgb(14 116 144);\">@" + tag + "</h1>";
          body += "<p>" + value + "</p>";
        }
      }
body += "<br/><br/></div>";
page.html = getPageTemplate(context, page.name, body);
}

void createTypePage(Statement::DefinedTypeDef* node, DocGenContext context, DocumentationPage& page) {
  std::string body = "";
  body += "<h1>";
  auto pathParts = page.path;
  auto nameParts = utils::list2vec(utils::split(page.name, "::"));
  int i = 0;
  for (auto _ : pathParts) {
    std::string url = "";
    int j = 0;
    for (auto part : pathParts) {
      if (j == i) {
        url += part.string();
        if (utils::endsWith(part.string(), ".html"))
          url = url.substr(0, url.size() - 5);
        break;
      } else {
        url += part.string() + "/";
      }
      j++;
    }
    body += "<a style=\"color:rgb(14 116 144);\" href=\"" + url + ".html\">" + nameParts[i] + "</a>::";
    i++;
  }
  body = body.substr(0, body.size() - 2) + ";"; // remove the last "::"
  body += "</h1><hr/><div><pre><code class=\"language-snowball\">";
  if (node->isPrivate()) body += "private ";
  if (node->isPublic()) body += "public ";
  if (node->isInterface()) body += "interface ";
  else if (node->isStruct()) body += "struct ";
  else body += "class ";
  body += node->getName();
  if (node->isGeneric()) {
    body += "&lt;";
    for (auto& generic : node->getGenerics()) {
      body += generic->getName();
      if (generic->getType()) {
        body += " = " + typeToHtml(generic->getType());
      }
      body += ", ";
    }
    // remove the last ", "
    if (node->getGenerics().size() > 0)
      body = body.substr(0, body.size() - 2);
    body += "&gt;";
  }
  std::vector<Statement::VariableDecl*> publicMembers;
  for (auto& field : node->getVariables()) {
    if (field->isPublic())
      publicMembers.push_back(field);
  }
  if (node->getParent()) {
    body += "\n    extends " + typeToHtml(node->getParent());
  }
  if (node->getImpls().size() > 0) {
    body += "\n    implements ";
    for (auto& impl : node->getImpls()) {
      body += typeToHtml(impl) + ", ";
    }
    // remove the last ", "
    body = body.substr(0, body.size() - 2);
  }
  if (publicMembers.size() > 0)
    body += " {\n  public:";
  else body += " {\n";
  for (auto& field : publicMembers) {
    if (field->getComment()) {
      auto values = field->getComment()->getValues();
      for (auto& [tag, value] : values) {
          if (tag == "brief") {
            body += "\n    // " + value;
            break;
          }
        }
      }
  body += "\n    " + (std::string)(field->isContantDecl() ? "const " : "let ") + field->getName() + ": " + typeToHtml(
              field->getDefinedType()) + ";\n";
  }
  if (utils::endsWith(body, ",\n"))
    body = body.substr(0, body.size() - 2);
  if (publicMembers.size() < node->getVariables().size())
    body += "  private:\n    // ... private fields\n";
  if (utils::endsWith(body, "{\n"))
    body = body.substr(0, body.size() - 1);
  body += "};</code></pre></div><br><div class=\"doc\">";
  if (auto docString = node->getComment()) {
    auto values = docString->getValues();
    if (values.count("brief")) {
      body += "<h1 style=\"color:rgb(14 116 144);\">Brief Description for <quote>" + node->getName() + "</quote></h1>";
      body += "<p>" + sanitize(values["brief"]) + "</p>";
    }
    body += "<p>" + sanitize(docString->getBody()) + "</p>";
    for (auto& [tag, value] : values) {
        if (tag == "brief") continue;
      }
    }
if (publicMembers.size() > 0) {
    body += "<hr/><h1 style=\"color:rgb(14 116 144);\">Public Fields</h1>";
    for (auto& field : publicMembers) {
      body += "<div style=\"display: grid; grid-template-columns: 1fr 1fr 1fr;\"><a href=\"" + page.path.string().substr(0,
              page.path.string().size() - 5) + "/" + field->getName() +
              "-var.html\"><h1 style=\"color:rgb(14 116 144);margin-right: 10px;font-weight: normal;\">" + field->getName();
      if (field->isContantDecl()) body += "<span class=\"tag\">static const</span>";
      body += "</h1></a>";
      if (field->getComment()) {
        auto values = field->getComment()->getValues();
        if (values.count("brief")) {
          auto brief = sanitize(values["brief"]);
          utils::replaceAll(brief, "<br/>", " ");
          body += "<span>" + brief + "</span>";
        }
      }
      body += "</div>";
    }
  }
  if (node->getFunctions().size() > 0) {
    std::vector<Statement::FunctionDef*> publicFunctions;
    std::vector<Statement::FunctionDef*> publicStaticFunctions;
    for (auto& func : node->getFunctions()) {
      if (func->isPublic()) {
        if (func->isStatic()) publicStaticFunctions.push_back(func);
        else publicFunctions.push_back(func);
      }
    }
    if (publicFunctions.size() > 0) {
      body += "<hr/><h1 style=\"color:rgb(14 116 144);\">Public Functions</h1>";
      for (auto& func : publicFunctions) {
        createSmallPictureFn(func, context, body);
      }
    }
    if (publicStaticFunctions.size() > 0) {
      body += "<h1 style=\"color:rgb(14 116 144);\">Public Static Functions</h1>";
      for (auto& func : publicStaticFunctions) {
        createSmallPictureFn(func, context, body);
      }
    }
    body += "<br/><br/><br/>";
  }
  body += "</div>";
  std::vector<std::pair<std::string, std::string>> links;
  for (auto& func : node->getFunctions()) {
    auto name = func->getName();
    bool isOperator = false;
    if (Operator::isOperator(name)) {
      isOperator = true;
      name = "-" + Operator::operatorName(Operator::operatorID(name));
    }
    std::string link = page.path.string();
    link = link.substr(0, link.size() - 5); // remove ".html"
    link += "/" + name;
    links.push_back({isOperator ? name.substr(1) : name, link});
  }
  page.html = getPageTemplate(context, page.name, body, links);
}

void createModulePage(std::vector<Syntax::Node*> nodes, DocGenContext context, DocumentationPage& page) {
  std::string body = "";
  body += "<h1>";
  auto pathParts = page.path;
  auto nameParts = utils::list2vec(utils::split(page.name, "::"));
  int i = 0;
  for (auto _ : pathParts) {
    std::string url = "";
    int j = 0;
    for (auto part : pathParts) {
      if (j == i) {
        url += part.string();
        if (utils::endsWith(part.string(), ".html"))
          url = url.substr(0, url.size() - 5);
        break;
      } else {
        url += part.string() + "/";
      }
      j++;
    }
    body += "<a style=\"color:rgb(14 116 144);\" href=\"" + url + ".html\">" + nameParts[i] + "</a>::";
    i++;
  }
  body = body.substr(0, body.size() - 2) + ";"; // remove the last "::"
  body += "</h1><hr/><div class=\"doc\">";
  GENERATE_TOP_LEVEL_NODES(nodes)
  body += "</div>";
  page.html = getPageTemplate(context, page.name, body);
}

void createMacroPage(Macro* node, DocGenContext context, DocumentationPage& page) {
  std::string body = "";
  body += "<h1>";
  auto pathParts = page.path;
  auto nameParts = utils::list2vec(utils::split(page.name, "::"));
  int i = 0;
  for (auto _ : pathParts) {
    std::string url = "";
    int j = 0;
    for (auto part : pathParts) {
      if (j == i) {
        url += part.string();
        if (utils::endsWith(part.string(), ".html"))
          url = url.substr(0, url.size() - 5);
        break;
      } else {
        url += part.string() + "/";
      }
      j++;
    }
    body += "<a style=\"color:rgb(14 116 144);\" href=\"" + url + ".html\">" + nameParts[i] + "</a>::";
    i++;
  }
  body = body.substr(0, body.size() - 2) + ";"; // remove the last "::"
  body += "</h1><hr/><div><pre><code class=\"language-snowball\">";
  body += "macro " + node->getName() + "(";
  for (auto& param : node->getArgs()) {
    body += std::get<0>(param);
    auto argType = std::get<1>(param);
    body += ": " + Macro::arguementTypeToSyntax(argType);
    if (std::get<2>(param) != nullptr) {
      body += " = ...";
    }
    body += ", ";
  }
  if (node->getArgs().size() > 0)
    body = body.substr(0, body.size() - 2);
  if (node->isMacroStatement())
    body += ") {\n    ...\n}";
  else body += ") = ...";
  body += "</code></pre></div><br><div class=\"doc\">";
  if (auto docString = node->getComment()) {
    auto values = docString->getValues();
    if (values.count("brief")) {
      body += "<h1 style=\"color:rgb(14 116 144);\">Brief Description for <quote>" + node->getName() + "</quote></h1>";
      body += "<p>" + sanitize(values["brief"]) + "</p>";
    }
    body += "<p>" + sanitize(docString->getBody()) + "</p>";
    for (auto& [tag, value] : values) {
        if (tag == "brief") continue;
      }
    }
body += "<br/><br/></div>";
page.html = getPageTemplate(context, page.name, body);
}

void createNamespacePage(Statement::Namespace* node, DocGenContext context, DocumentationPage& page) {
  std::string body = "";
  body += "<h1>";
  auto pathParts = page.path;
  auto nameParts = utils::list2vec(utils::split(page.name, "::"));
  int i = 0;
  for (auto _ : pathParts) {
    std::string url = "";
    int j = 0;
    for (auto part : pathParts) {
      if (j == i) {
        url += part.string();
        if (utils::endsWith(part.string(), ".html"))
          url = url.substr(0, url.size() - 5);
        break;
      } else {
        url += part.string() + "/";
      }
      j++;
    }
    body += "<a style=\"color:rgb(14 116 144);\" href=\"" + url + ".html\">" + nameParts[i] + "</a>::";
    i++;
  }
  body = body.substr(0, body.size() - 2) + ";"; // remove the last "::"
  body += "</h1><hr/><div class=\"doc\">";
  body += "<pre><code class=\"language-snowball\">";
  body += "namespace " + node->getName() + " {\n";
  body += " // ...\n";
  body += "};</code></pre><br/><br/>";
  if (auto docString = node->getComment()) {
    auto values = docString->getValues();
    if (values.count("brief")) {
      body += "<h1 style=\"color:rgb(14 116 144);\">Brief Description for <quote>" + node->getName() + "</quote></h1>";
      body += "<p>" + sanitize(values["brief"]) + "</p>";
    }
    body += "<p>" + sanitize(docString->getBody()) + "</p>";
    for (auto& [tag, value] : values) {
        if (tag == "brief") continue;
        body += "<hr/><h1 style=\"color:rgb(14 116 144);\">@" + tag + "</h1>";
        body += "<p>" + sanitize(value) + "</p>";
      }
    }
GENERATE_TOP_LEVEL_NODES(node->getBody())
  body += "<br/><br/></div>";
  page.html = getPageTemplate(context, page.name, body);
}

void createVariablePage(Statement::VariableDecl* node, DocGenContext context, DocumentationPage& page) {
  std::string body = "";
  body += "<h1>";
  auto pathParts = page.path;
  auto nameParts = utils::list2vec(utils::split(page.name, "::"));
  int i = 0;
  for (auto _ : pathParts) {
    std::string url = "";
    int j = 0;
    for (auto part : pathParts) {
      if (j == i) {
        url += part.string();
        if (utils::endsWith(part.string(), ".html"))
          url = url.substr(0, url.size() - 5);
        break;
      } else {
        url += part.string() + "/";
      }
      j++;
    }
    body += "<a style=\"color:rgb(14 116 144);\" href=\"" + url + ".html\">" + nameParts[i] + "</a>::";
    i++;
  }
  body = body.substr(0, body.size() - 2) + ";"; // remove the last "::"
  if (!context.currentType.empty()) {
    if (node->isContantDecl())
      body += "<span class=\"tag\">static object constant</span> ";
    else body += "<span class=\"tag\">object field</span> ";
  }
  body += "</h1><hr/><div><pre><code class=\"language-snowball\">";
  auto varName = node->getName();
  if (!context.currentType.empty()) {
    varName = "Self::" + varName;
  }
  body += (std::string)(node->isContantDecl() ? "const " : "let ") + varName;
  if (node->getDefinedType()) body += ": " + typeToHtml(node->getDefinedType());
  body += " = ...";
  body += "</code></pre></div><br><div class=\"doc\">";
  if (auto docString = node->getComment()) {
    auto values = docString->getValues();
    if (values.count("brief")) {
      body += "<h1 style=\"color:rgb(14 116 144);\">Brief Description for <quote>" + node->getName() + "</quote></h1>";
      body += "<p>" + sanitize(values["brief"]) + "</p>";
    }
    body += "<p>" + sanitize(docString->getBody()) + "</p>";
    for (auto& [tag, value] : values) {
        if (tag == "brief") continue;
      }
    }
body += "<br/><br/></div>";
page.html = getPageTemplate(context, page.name, body);
}

void createRootPage(std::vector<std::string> modules, DocGenContext context, DocumentationPage& page) {
  std::string body = "";
  body += "<h1>Modules in " + context.currentModule + "</h1><hr/><div class=\"doc\">";
  std::map<std::string, std::vector<std::string>> moduleMap;
  for (auto& module : modules) {
    auto parts = utils::list2vec(utils::split(module, "/"));
    if (parts.size() > 1) {
      moduleMap[parts[0]].push_back(parts[1]);
    } else {
      moduleMap[""].push_back(parts[0]);
    }
  }
  // create a list, if the key is not empty, we create a sub-list
  for (auto& [key, value] : moduleMap) {
      if (!key.empty()) {
        body += "<br/><h1 style=\"font-size: 25px;\">Modules in " + key + "</h1><br/>";
      }
    for (auto& module : value) {
      auto moduleUrl = page.path.string().substr(0,
                         page.path.string().size() - 5) + "/" + (!key.empty() ? key + "/" : "") + module + ".html";
        body += "<div style=\"display: grid; grid-template-columns: 1fr 1fr 1fr;\"><a href=" + moduleUrl +
                "><h1 style=\"color:rgb(14 116 144);margin-right: 10px;font-weight: normal;\">" + module + "</h1></a>";
        body += "</div>";
      }
  }
  page.html = getPageTemplate(context, page.name, body);
}

void createTypeAliasPage(Statement::TypeAlias* node, DocGenContext context, DocumentationPage& page) {
  std::string body = "";
  body += "<h1>";
  auto pathParts = page.path;
  auto nameParts = utils::list2vec(utils::split(page.name, "::"));
  int i = 0;
  for (auto _ : pathParts) {
    std::string url = "";
    int j = 0;
    for (auto part : pathParts) {
      if (j == i) {
        url += part.string();
        if (utils::endsWith(part.string(), ".html"))
          url = url.substr(0, url.size() - 5);
        break;
      } else {
        url += part.string() + "/";
      }
      j++;
    }
    body += "<a style=\"color:rgb(14 116 144);\" href=\"" + url + ".html\">" + nameParts[i] + "</a>::";
    i++;
  }
  body = body.substr(0, body.size() - 2) + ";"; // remove the last "::"
  body += "<span class=\"tag\">type alias</span> ";
  body += "</h1><hr/><div><pre><code class=\"language-snowball\">";
  body += "type " + node->getIdentifier() + " = " + typeToHtml(node->getType());
  body += "</code></pre></div><br><div class=\"doc\">";
  if (auto docString = node->getComment()) {
    auto values = docString->getValues();
    if (values.count("brief")) {
      body += "<h1 style=\"color:rgb(14 116 144);\">Brief Description for <quote>" + node->getIdentifier() + "</quote></h1>";
      body += "<p>" + sanitize(values["brief"]) + "</p>";
    }
    body += "<p>" + sanitize(docString->getBody()) + "</p>";
    for (auto& [tag, value] : values) {
        if (tag == "brief") continue;
      }
    }
body += "<br/><br/></div>";
page.html = getPageTemplate(context, page.name, body);
}

} // namespace docgen
} // namespace Syntax
} // namespace snowball
