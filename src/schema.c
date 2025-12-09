static const sql_FieldSchema schema_customers_fields[] = {
    {.name = "id", .type = "integer primary key autoincrement", .sqlite_backing_type = SQLITE_INTEGER},
    {.name = "name", .type = "text", .sqlite_backing_type = SQLITE_TEXT},
    {.name = "address_line1", .type = "text", .sqlite_backing_type = SQLITE_TEXT},
    {.name = "address_line2", .type = "text", .sqlite_backing_type = SQLITE_TEXT},
    {.name = "city", .type = "text", .sqlite_backing_type = SQLITE_TEXT},
    {.name = "street", .type = "text", .sqlite_backing_type = SQLITE_TEXT},
};
static const sql_TableSchema schema_customers = {
    .fields = schema_customers_fields,
    .nfields = CORE_ARRAY_LEN(schema_customers_fields),
    .name = "customers"
};

static const sql_TableSchema schemas[] = {
    schema_customers
};
    
