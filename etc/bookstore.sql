-- structure
create table books (
	id integer primary key autoincrement,
	title text not null,
	author text nut null,
	isbn text,
	price real,
	available integer check (available >= 0)
); 

create table employees (
	id integer primary key autoincrement,
	fullname text not null,
	position text,
	birthdate date,
	phone text,
	email text,
	comment	text
);

create table customers (
	id integer primary key autoincrement,
	fullname text not null,
	address text,
	phone text,
	email text
);

create table orders (
	id integer primary key autoincrement,
	customer_id integer,
	employee_id integer,
	sale_date date,
	comment text,
	foreign key(customer_id) references customers(id),
	foreign key(employee_id) references employees(id)	
);

create table order_books (
	id integer primary key autoincrement,
	order_id integer,
	book_id integer,
	quantity integer,
	discount real,
	foreign key(order_id) references orders(id),
	foreign key(book_id) references books(id)
);

create table reviews (
	id integer primary key autoincrement,
	customer_id int not null,
	book_id int not null,
	rating int,
	review text,
	check(rating is not null and rating >= 1 and rating <= 5 or rating is null and review is not null),
	unique(customer_id, book_id),
	foreign key(customer_id) references customers(id),
	foreign key(book_id) references books(id)
);

create view v_bestseller5 as
select b.author, b.title, sum(ob.quantity) cnt
from order_books ob inner join books b on ob.book_id == b.id
group by book_id
order by 3 desc
limit 5;

create view v_popular5 as
select b.author, b.title, printf("%.2f", avg(r.rating)) rating, count(1) review_count
from reviews r inner join books b on r.book_id == b.id
group by r.book_id
order by 4 desc, 3 desc
limit 5;

-- data	
insert into books (id, title, author, isbn, price, available) values
(1, 'Steelheart', 'Brandon Sanderson', '978-0385743563', 5, 3),
(2, 'All Our Yesterdays', 'Cristin Terrill', '978-1423176374', 7.3, 10),
(3, 'Life After Life', 'Jill McCorkle', '978-1616203221', 2.82, 4),
(4, 'Life After Life', 'Kate Atkinson', '978-0316176491', 8.1, 5),
(5, 'Foundation', 'Isaac Asimov', '978-0553293357', 12.7, 8),
(6, 'Dracula', 'Bram Stoker', '978-0486411095', 6.6, 0),
(7, 'Ordinary Grace', 'William Kent Krueger', '978-1451645859', 3.2, 9),
(8, 'The Nightingale', 'Kristin Hannah', '978-0312577223', 11, 1),
(9, 'The Life We Bury', 'Allen Eskens', '978-1616149987', 5.1, 4),
(10, 'Take Me With You', 'Catherine Ryan Hyde', '978-1477820018', 0.99, 22),
(11, 'Secrets of a Charmed Life', 'Susan Meissner', '978-0451419927', 3.2, 8),
(12, 'The Silent Sister', 'Diane Chamberlain', '978-1250074355', 8.7, 3),
(13, 'Everything I Never Told You', 'Celeste Ng', '978-0143127550', 1.99, 2),
(14, 'What She Left Behind', 'Ellen M. Wiseman', '978-0758278456', 5.5, 10),
(15, 'Trail of Broken Wings', 'Sejal Badani', '978-1477822081', 9.8, 2),
(16, 'The Girl on the Train', 'Paula Hawkins', '978-1594633669', 99.9, 15),
(17, 'The Way We Fall', 'Cassia Leo', '978-1507704530', 18.1, 0),
(18, 'When I Found You', 'Catherine Ryan Hyde', '978-1611099799', 16.1, 1),
(19, 'The Narrow Road to the Deep North', 'Richard Flanagan', '978-0804171472', 7.8, 6),
(20, 'The Light Between Oceans', 'M.L. Stedman', '978-1451681758', 14.5, 32),
(21, 'The Invention of Wings', 'Sue Monk Kidd', '978-0143121701', 17, 11),
(22, 'Between the World and Me', 'Ta-Nehisi Coates', '978-0812993547', 8.12, 2),
(23, 'When Breath Becomes Air', 'Deckle Edge', '978-0812988406', 7.8, 5),
(24, 'The Good Neighbor', 'A. J. Banner', '978-1503944435', 44.4, 1),
(25, 'The Pecan Man', 'Cassie D. Selleck', '978-0615590585', 6.1, 0),
(26, 'The Martian', 'Andy Weir', '978-0553418026', 9.7, 7),
(27, 'A Long Walk to Water', 'Linda Sue Park', '978-0547577319', 5.4, 2),
(28, 'The Alchemist', 'Paulo Coelho', '978-0062315007', 6.2, 4),
(29, 'A Fall of Marigolds', 'Susan Meissner', '978-0451419910', 1.7, 3),
(30, 'What She Knew', 'Gilly Macmillan', '978-0062413864', 6.4, 2),
(31, 'Fates and Furies', 'Lauren Groff', '978-1594634475', 44.7, 1),
(32, 'Wool', 'Hugh Howey', '978-1476733951', 3.39, 10),
(33, 'The Plum Tree', 'Ellen M. Wiseman', '978-0758278432', 7.5, 2);

insert into employees (id, fullname, position, birthdate, phone, email, comment) values
(1, 'Kolin McGregor', 'administrator', '1976-12-11', '555-56-65', 'kolin.mcgregor@bestbuy.com', 'Founder'),
(2, 'Suzzi Rebba', 'manager', '1987-04-25', '555-11-02', 'suzzi.rebba@bestbuy.com', null),
(3, 'Mel Braun', 'manager', '1955-07-01', '555-09-25', 'mel.braun@bestbuy.com', 'CoFounder'),
(4, 'Tommy Lee', 'helper', '1992-07-01', '555-03-01', null, null),
(5, 'Amma Honk', 'accountant', '1974-02-22', '555-55-05', 'amma.honk@bestbuy.com', null);

insert into customers (id, fullname, address, phone, email) values
(1, 'Bran Oldy', '567 College Lane Warren, MI 48089', '555-12-13', 'oldy@supreme.org'),
(2, 'Tomas Krave', '488 West Cobblestone St. Waxhaw, NC 28173', '555-01-34', 'tomas@mydoggy.com'),
(3, 'Kate Broom', '87 Old Glen Ridge St. Feasterville Trevose, PA 19053', '555-99-03', 'kitty87@mail.com'),
(4, 'Harry Port', '83 Valley Lane Clayton, NC 27520', '555-11-11', 'h.port@gmail.com'),
(5, 'Sam Jackson', '446 Clinton Rd. Bayonne, NJ 07002', '555-00-87', 'samjack@topdone.org'),
(6, 'Alex Brown', '4 Rockaway St. Wellington, FL 33414', '555-87-55', 'brown@bookshop.en'),
(7, 'Elle Noika', '670 Santa Clara Road Sheboygan, WI 53081', '555-99-44', 'elle@camp56.org'),
(8, 'Mitchel Holt', '26 Lower River Dr. Romeoville, IL 60446', '555-76-76', 'mitchel@camp56.org'),
(9, 'Drew Watson', '44 Old Buckingham Lane Drexel Hill, PA 19026', '555-43-43', 'watson@bing.to');

insert into orders (id, customer_id, employee_id, sale_date, comment) values 
(1, 1, 2, '2020-09-11', null),
(2, 2, 2, '2020-09-11', null),
(3, 3, 3, '2020-09-12', null),
(4, 4, 3, '2020-09-12', null),
(5, 8, 4, '2020-09-13', null),
(6, 1, 2, '2020-09-13', null),
(7, 8, 4, '2020-09-13', null),
(8, 7, 2, '2020-09-20', null),
(9, 6, 2, '2020-09-21', null),
(10, 6, 2, '2020-09-21', 'Awaiting payment'),
(11, 5, 1, '2020-09-22', null),
(12, 1, 2, '2020-09-22', null),
(13, 3, 3, '2020-09-23', null),
(14, 8, 3, '2020-09-23', null),
(15, 1, 3, '2020-09-24', null),
(16, 8, 2, '2020-09-27', null),
(17, 7, 2, '2020-09-27', null),
(18, 6, 1, '2020-09-28', null),
(19, 6, 2, '2020-10-01', 'Delivery problem');

insert into order_books (id, order_id, book_id, quantity, discount) values
(1, 1, 10, 2, 0),
(2, 1, 11, 1, 0),
(3, 2, 33, 1, 0),
(4, 3, 2, 1, 0),
(5, 3, 3, 1, 0),
(6, 4, 3, 2, 0),
(7, 4, 10, 3, 2),
(8, 4, 5, 1, 0),
(9, 5, 10, 1, 0),
(10, 6, 17, 1, 0),
(11, 6, 16, 1, 0),
(12, 7, 1, 1, 0),
(13, 8, 2, 2, 0),
(14, 9, 10, 3, 2),
(15, 10, 27, 2, 0),
(16, 11, 32, 1, 0),
(17, 11, 10, 1, 0),
(18, 11, 17, 1, 0),
(19, 12, 11, 2, 0),
(20, 12, 1, 1, 0),
(21, 13, 1, 2, 0),
(22, 14, 1, 1, 0),
(23, 15, 1, 1, 0),
(24, 16, 1, 1, 0),
(25, 16, 19, 4, 4),
(26, 17, 12, 2, 0),
(27, 17, 10, 1, 0),
(28, 18, 11, 1, 0),
(29, 19, 7, 1, 0);

insert into reviews (id, customer_id, book_id, rating, review) values
(1, 6, 7, 5, null),
(2, 2, 5, 5, null),
(3, 3, 11, 5, 'Best book ever, OMG.'),
(4, 2, 33, 2, null),
(5, 1, 11, 5, 'Love it'),
(6, 7, 7, 3, 'Not very interesting.'),
(7, 8, 10, 5, null),
(8, 8, 11, 5, 'fascinating'),
(9, 9, 10, 4, 'Interesting subject'),
(10, 1, 5, 1, null),
(11, 3, 19, 5, 'so good'),
(12, 3, 15, 5, null),
(13, 2, 17, 2, 'bored'),
(14, 5, 11, 3, 'too confusing'),
(15, 5, 5, 5, null),
(16, 7, 8, 4, null),
(17, 8, 23, 5, 'very good'),
(18, 8, 7, 5, null),
(19, 8, 4, 1, 'I do not like it at all'),
(20, 9, 11, 5, null),
(21, 7, 18, 3, 'not very interesting'),
(22, 7, 32, 5, null),
(23, 6, 6, 5, 'the best book ever'),
(24, 5, 4, 5, 'nice book'),
(25, 2, 25, 5, null),
(26, 2, 8, 1, 'worst book ever'),
(27, 3, 7, 5, 'amazing book'),
(28, 3, 4, 5, 'it is the best book I have ever read'),
(29, 4, 33, 5, null),
(30, 5, 18, 2, 'so boring'),
(31, 4, 24, 2, null),
(32, 4, 12, 2, null),
(33, 9, 2, 3, 'too long to read'),
(34, 9, 1, 4, null),
(35, 8, 1, 5, 'interesting story');