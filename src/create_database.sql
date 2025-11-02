-- Turn on foreign key checking so relationships between tables are enforced.
PRAGMA foreign_keys = ON;

BEGIN TRANSACTION;
-- We start a transaction so the whole schema creation either all succeeds or all fails.
-- (This keeps the database consistent if something errors while creating the tables.)

-- -------------------------
-- 1) Customers table
-- -------------------------
-- This table stores one row per customer (company). Think: company name, address, primary contact, phone, etc.
CREATE TABLE customers (
    id              INTEGER PRIMARY KEY AUTOINCREMENT, -- internal unique id for each customer
    name            TEXT NOT NULL,                     -- company or customer name (required)
    tax_id          TEXT,                              -- optional tax or VAT id
    address_line1   TEXT,                              -- street address (line 1)
    address_line2   TEXT,                              -- street address (line 2, optional)
    city            TEXT,
    state           TEXT,
    postal_code     TEXT,
    country         TEXT,
    phone           TEXT,
    email           TEXT,
    website         TEXT,
    primary_contact_name  TEXT,                        -- a person to contact at that company
    primary_contact_phone TEXT,
    primary_contact_email TEXT,
    notes           TEXT,                              -- free-form notes about the customer
    created_at      DATETIME DEFAULT (datetime('now')),-- when this row was created
    updated_at      DATETIME                           -- last update time (set by your app)
);

-- Index: speeds up searches by customer name (useful for lookups)
CREATE INDEX idx_customers_name ON customers(name);

-- -------------------------
-- 2) Parts master table
-- -------------------------
-- This holds a row for each part we make or stock. It's the "master list" of part numbers and descriptions.
CREATE TABLE parts (
    part_number     TEXT PRIMARY KEY,     -- human part number e.g. "ABC-123" (unique)
    description     TEXT,                 -- human-readable description of the part
    unit            TEXT,                 -- unit of measure (e.g. "each", "set")
    weight          REAL,                 -- optional numeric weight
    default_location TEXT,                -- default storage location name
    supplier        TEXT,                 -- who supplies this part (if applicable)
    lead_time_days  INTEGER,              -- how many days to make/receive from supplier
    created_at      DATETIME DEFAULT (datetime('now')),
    updated_at      DATETIME
);

CREATE INDEX idx_parts_description ON parts(description);

-- -------------------------
-- 3) Orders table (open + archive flag)
-- -------------------------
-- Each row is a customer order (one order can have many parts / line items).
-- We keep both open and archived orders in this table; if archived_at is set it is completed.
CREATE TABLE orders (
    id                      INTEGER PRIMARY KEY AUTOINCREMENT, -- internal id
    customer_id             INTEGER NOT NULL REFERENCES customers(id) ON DELETE CASCADE,
        -- links to the customers table. 'ON DELETE CASCADE' means if a customer is deleted,
        -- their orders are removed too.
    customer_order_number   TEXT NOT NULL,   -- the order number the customer gave us (their PO number)
    order_date              DATE DEFAULT (date('now')), -- date the order was created/received
    status                  TEXT NOT NULL DEFAULT 'open',  -- lightweight status text
    total_amount            REAL,                -- optional order value
    currency                TEXT DEFAULT 'USD',
    notes                   TEXT,
    archived_at             DATETIME,            -- if NOT NULL -> this order is archived / completed
    created_at              DATETIME DEFAULT (datetime('now')),
    updated_at              DATETIME
);

CREATE INDEX idx_orders_customer ON orders(customer_id);
CREATE INDEX idx_orders_customer_ordernum ON orders(customer_order_number);
CREATE INDEX idx_orders_status ON orders(status);

-- -------------------------
-- 4) Order documents (file storage)
-- -------------------------
-- Use this to store files customers sent (PDFs, images). We keep file content in a BLOB column.
CREATE TABLE order_documents (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    order_id        INTEGER NOT NULL REFERENCES orders(id) ON DELETE CASCADE, -- which order this file belongs to
    file_name       TEXT,                   -- original filename e.g. "PO_1234.pdf"
    mime_type       TEXT,                   -- e.g. "application/pdf"
    content         BLOB,                   -- raw file bytes (binary large object)
    uploaded_by     TEXT,
    uploaded_at     DATETIME DEFAULT (datetime('now'))
);

CREATE INDEX idx_order_documents_order_id ON order_documents(order_id);

-- -------------------------
-- 5) Order line items (open parts / parts to run)
-- -------------------------
-- Each row is a single line on an order: part number, quantity, manufacturing status, due date, etc.
CREATE TABLE order_items (
    id                      INTEGER PRIMARY KEY AUTOINCREMENT,
    order_id                INTEGER NOT NULL REFERENCES orders(id) ON DELETE CASCADE,
    part_number             TEXT REFERENCES parts(part_number) ON DELETE SET NULL,
        -- references the parts table. ON DELETE SET NULL means if a part is removed from the master list,
        -- the order item keeps its row but the part_number becomes NULL (so historic orders remain).
    customer_line_number    TEXT,            -- the line number from the customer's PO (optional)
    quantity                INTEGER NOT NULL DEFAULT 1, -- how many of this part the customer ordered
    quantity_completed      INTEGER NOT NULL DEFAULT 0, -- how many produced so far
    unit_price              REAL,            -- optional price per unit (if you track it)
    delivery_method         TEXT,            -- how the part will be delivered: ship/pickup/etc.
    manufacturing_status    TEXT NOT NULL DEFAULT 'not_started',
        -- e.g. 'not_started', 'running', 'completed'. Your app should keep this consistent.
    due_date                DATE,            -- when the part should be finished/delivered
    notes                   TEXT,
    created_at              DATETIME DEFAULT (datetime('now')),
    updated_at              DATETIME
);

CREATE INDEX idx_order_items_order_id ON order_items(order_id);
CREATE INDEX idx_order_items_part_number ON order_items(part_number);
CREATE INDEX idx_order_items_status ON order_items(manufacturing_status);

-- -------------------------
-- 6) Inventory table
-- -------------------------
-- Where parts are physically stored and how many are on hand.
-- Primary key is (part_number, location, bin) so you can have multiple bins per part.
CREATE TABLE inventory (
    part_number         TEXT NOT NULL REFERENCES parts(part_number) ON DELETE CASCADE,
    location            TEXT NOT NULL,      -- e.g. "Main Warehouse" or building name
    bin                 TEXT,               -- shelf/bin label, optional
    quantity_on_hand    INTEGER NOT NULL DEFAULT 0, -- how many physically in this bin
    reserved_quantity   INTEGER NOT NULL DEFAULT 0, -- how many are reserved for orders/in-process
    minimum_level       INTEGER DEFAULT 0,
    last_counted_at     DATETIME,
    notes               TEXT,
    PRIMARY KEY (part_number, location, bin)
);

CREATE INDEX idx_inventory_part ON inventory(part_number);

-- -------------------------
-- 7) Helpful views (saved queries)
-- -------------------------
-- Views are like saved SELECT statements. They make common queries simpler.

-- Open orders (orders that are NOT archived)
CREATE VIEW view_open_orders AS
SELECT o.*
FROM orders o
WHERE o.archived_at IS NULL
ORDER BY o.order_date DESC, o.id;

-- Parts still to run: order_items for orders that are not archived and that are not marked 'completed'
CREATE VIEW view_parts_to_run AS
SELECT oi.*, o.customer_order_number, o.customer_id, p.description AS part_description
FROM order_items oi
JOIN orders o ON o.id = oi.order_id
LEFT JOIN parts p ON p.part_number = oi.part_number
WHERE o.archived_at IS NULL
  AND (oi.manufacturing_status IS NULL OR lower(oi.manufacturing_status) != 'completed')
ORDER BY oi.due_date IS NULL, oi.due_date;

-- Inventory summary: totals per part across all locations
CREATE VIEW view_inventory_summary AS
SELECT part_number, SUM(quantity_on_hand) AS total_on_hand, SUM(reserved_quantity) AS total_reserved
FROM inventory
GROUP BY part_number;

COMMIT;
-- End of schema creation
