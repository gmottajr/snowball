
#include "sourceInfo/DBGSourceInfo.h"

#include "sourceInfo/SourcedObject.h"

#include <sstream>
#include <string>

namespace snowball {
DBGSourceInfo::DBGSourceInfo(const SourceInfo* p_source_info, uint32_t p_line)
  : SrcObject(p_source_info), line(p_line) { }

DBGSourceInfo::DBGSourceInfo(const SourceInfo* p_source_info, std::pair<int, int> p_pos, uint32_t p_width)
  : pos(p_pos), line((uint32_t) p_pos.first), width(p_width), SrcObject(p_source_info) { }

void DBGSourceInfo::prepare_for_error() {
  uint64_t cur_line = 1;
  line_before.clear();
  line_before_before.clear();
  line_str.clear();
  line_after.clear();
  line_after_after.clear();
  const auto& source = m_srci->getSource();
  for (auto c : source) {
    if (c == '\n') {
      if (cur_line >= line + 2) break;
      cur_line++;
    } else if (cur_line == line - 2) {
      line_before_before += c;
    } else if (cur_line == line - 1) {
      line_before += c;
    } else if (cur_line == line) {
      line_str += c;
    } else if (cur_line == line + 1) {
      line_after += c;
    } else if (cur_line == line + 2) {
      line_after_after += c;
    }
  }
}

std::string DBGSourceInfo::get_pos_str() const {
  // var x = blabla;
  // ^^^^^^
  std::stringstream ss_pos;
  size_t cur_col = 0;
  bool done = false;
  for (size_t i = 0; i < line_str.size(); i++) {
    cur_col++;
    if (cur_col == (size_t)pos.second) {
      for (uint32_t i = 0; i < width; i++) { ss_pos << '~'; }
      done = true;
      break;
    } else if (line_str[i] != '\t') {
      ss_pos << ' ';
    } else {
      ss_pos << '\t';
    }
  }
  if (!done) ss_pos << '^';
  auto ret = ss_pos.str();
  return ret;
}

DBGObject::DBGObject() { /* noop */
}
DBGSourceInfo* DBGObject::getDBGInfo() { return dbg; }
void DBGObject::setDBGInfo(DBGSourceInfo* i) { dbg = i; }
} // namespace snowball