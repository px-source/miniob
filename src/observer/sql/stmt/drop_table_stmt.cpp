#include "sql/stmt/drop_table_stmt.h"
#include "storage/db/db.h"

RC DropTableStmt::create(Db *db, const DropTableSqlNode &drop_table, Stmt *&stmt)
{
    if(drop_table.relation_name.empty()) {
        return RC::INVALID_ARGUMENT;
    }
    stmt = new DropTableStmt(drop_table.relation_name);
    return RC::SUCCESS;
}

