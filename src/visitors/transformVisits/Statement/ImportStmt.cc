#include "../../../lexer.h"
#include "../../../parser/Parser.h"
#include "../../Analyzer.h"
#include "../../TransformState.h"
#include "../../Transformer.h"
#include "../../TypeChecker.h"
#include "../../analyzers/DefinitveAssigment.h"

#include <fstream>
#include <tuple>

using namespace snowball::utils;
using namespace snowball::Syntax::transform;

namespace snowball {
namespace Syntax {

SN_TRANSFORMER_VISIT(Statement::ImportStmt) {
    auto package = p_node->getPackage();
    auto path = p_node->getPath();
    // TODO: extension
    auto [filePath, error] = ctx->imports->getImportPath(package, path);
    if (!error.empty()) { E<IO_ERROR>(p_node, error); }
    auto uuid = package == "Core" ? ctx->imports->CORE_UUID + path[0]
                                  : ctx->imports->getModuleUUID(filePath);
    auto exportName = ctx->imports->getExportName(filePath, p_node->getExportSymbol());
    if (ctx->getItem(exportName).second)
        Syntax::E<IMPORT_ERROR>(
                p_node, FMT("Import with name '%s' is already defined!", exportName.c_str()));
    if (auto m = ctx->imports->cache->getModule(filePath)) {
        auto item = std::make_shared<Item>(m.value());
        ctx->addItem(exportName, item);
    } else {
        auto niceFullName = package + "::" + utils::join(path.begin(), path.end(), "::");
        auto mod = std::make_shared<ir::Module>(niceFullName, uuid);
        auto st = std::make_shared<ContextState::StackType>();
        auto state = std::shared_ptr<ContextState>(new ContextState(st, mod, nullptr));
        // clang-format off
        ctx->withState(state,
            [filePath = filePath, this]() mutable {
                std::ifstream ifs(filePath.string());
                assert(!ifs.fail());
                std::string content((std::istreambuf_iterator<char>(ifs)),
                                    (std::istreambuf_iterator<char>()));
                auto srcInfo = new SourceInfo(content, filePath);
                auto lexer = new Lexer(srcInfo);
                lexer->tokenize();
                auto tokens = lexer->tokens;
                if (tokens.size() != 0) {
                    auto parser = new parser::Parser(tokens, srcInfo);
                    auto ast = parser->parse();
                    ctx->module->setSourceInfo(srcInfo);
                    visit(ast);
                    // TODO: make this a separate function to avoid any sort of "conflict" with the compiler's version of this algorithm
                    std::vector<Syntax::Analyzer *> passes = {
                        new Syntax::DefiniteAssigment(srcInfo)};
                    for (auto pass : passes)
                        pass->run(ast);
                    auto typeChecker = new codegen::TypeChecker(ctx->module);
                    typeChecker->codegen();

                    // TODO: set a new module to the import cache
                    addModule(ctx->module);
                    ctx->imports->cache->addModule(filePath, ctx->module);
                }
        });
        // clang-format on
        auto item = std::make_shared<Item>(mod);
        ctx->addItem(exportName, item);
    }
}

} // namespace Syntax
} // namespace snowball