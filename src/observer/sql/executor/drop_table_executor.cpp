#include "sql/executor/drop_table_executor.h"
#include "event/session_event.h"
#include "event/sql_event.h"
#include "sql/stmt/drop_table_stmt.h"
#include "storage/db/db.h"
#include "session/session.h"

RC DropTableExecutor::execute(SQLStageEvent *sql_event)
{
    Stmt *stmt = sql_event->stmt();
    DropTableStmt *drop_stmt = static_cast<DropTableStmt *>(stmt);

    Session *session = sql_event->session_event()->session();
    Db *db = session->get_current_db();
    
    return db->drop_table(drop_stmt->table_name().c_str());
}