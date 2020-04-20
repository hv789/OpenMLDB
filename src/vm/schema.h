/*-------------------------------------------------------------------------
 * Copyright (C) 2020, 4paradigm
 * schema.h
 *
 * Author: chenjing
 * Date: 2020/4/20
 *--------------------------------------------------------------------------
 **/

#ifndef SRC_VM_SCHEMA_H_
#define SRC_VM_SCHEMA_H_
#include <map>
#include <string>
#include <utility>
#include <vector>
#include "node/sql_node.h"
#include "vm/catalog.h"
namespace fesql {
namespace vm {
struct RowSchemaInfo {
    const uint32_t idx;
    const std::string table_name_;
    const vm::Schema* schema_;
};
class SchemasContext {
 public:
    SchemasContext(
        const std::vector<std::pair<const std::string, const vm::Schema*>>&
            table_schema_list);

    bool ExprListResolved(std::vector<node::ExprNode*> expr_list,
                          const RowSchemaInfo** info) const;
    bool ExprRefResolved(const node::ExprNode* expr,
                         const RowSchemaInfo** info) const;

    bool AllRefResolved(const std::string& relation_name,
                        const RowSchemaInfo** info) const;
    bool ColumnRefResolved(const std::string& relation_name,
                           const std::string& col_name,
                           const RowSchemaInfo** info) const;

 public:
    // row ir context list
    std::vector<RowSchemaInfo> row_schema_info_list_;
    // column_name -> [context_id1, context_id2]
    std::map<std::string, std::vector<uint32_t>> col_context_id_map_;
    // table_name -> context_id1
    std::map<std::string, uint32_t> table_context_id_map_;
};
}  // namespace vm
}  // namespace fesql

#endif  // SRC_VM_SCHEMA_H_
