create table people (
       personid integer primary key autoincrement,
       firstname text not null,
       lastname text not null,
       email text,
       phone text
);

create table customer (
       name text primary key not null,
       contacts personid [] not null,
       address text not null
);

/*insert into people (firstname, lastname) values ('john', 'smith');*/
