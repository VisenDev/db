create table people (
       id integer primary key autoincrement,
       firstname text not null,
       lastname text not null,
       email text,
       phone text
);

insert into people (firstname, lastname) values ('john', 'smith');
