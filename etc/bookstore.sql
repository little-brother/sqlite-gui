-- based on https://github.com/SkylerKidd/SQLite_Bookstore
create table user (
	username text not null,
	email text not null,
	password text not null,
	fname text,
	minit char,
	lname text,
	bdate date,
	zipcode text,
	state text,
	city text,
	staddress text,
	primary key(username)
);

create table book (
	isbn text not null,
	edition text,
	title text not null,
	authorname text not null,
	primary key(isbn)
);

create table listing (
	seller text not null,
	book text not null,
	price real not null,
	quantity int,
	check (quantity >= 0),
	primary key(seller, book),
	foreign key(book) references book(isbn) on update cascade,
	foreign key(seller) references user(username) on update cascade on delete restrict
);

create table user_review (
	reviewer text not null,
	reviewee text not null,
	review text,
	rating int not null,
	check (rating >= 1 or rating <= 5),
	primary key(reviewer, reviewee),
	foreign key(reviewer) references user(username) on update cascade,
	foreign key(reviewee) references user(username) on update cascade
);

create table book_review (
	reviewer text not null,
	book text not null,
	review text,
	rating int not null,
	check (rating >= 1 or rating <= 5),
	primary key(reviewer, book),
	foreign key(reviewer) references user(username) on update cascade,
	foreign key(book) references book(isbn) on update cascade
);

create table transact (
	buyerun text not null,
	sellerun text not null,
	bookid text not null,
	datetime text not null,
	primary key(buyerun, sellerun, bookid, datetime),
	foreign key(buyerun) references user(username) on update cascade on delete restrict,
	foreign key(sellerun,bookid) references listing(seller, book) on update cascade on delete restrict
);

insert into user values('skyler','f.s.kidd@gmail.com','p@ssw0rd','Frank','S','Kidd','1995-02-10','98011','WA','Bothell','1234 Steeterson Street');
insert into user values('testerT','tester@example.com','12341234','Test','R','Testerson','1990-06-23','90210','WA','Beverly Hills','555 Richguy Blvd');
insert into user values('smithg','smithg@yahoo.com','11112222','John','R','Smith','1995-06-21','40712','FL','Orlando','85 Maple Tree Blvd');
insert into user values('AliG','alitheg@gmail.com','55556666','Ali','D','Gee','1985-03-23','40715','FL','Daytona Beach','18 Breakers Ave');
insert into user values('jamesB','jamesbb@hotmail.com','15151616','James','R','Brown','1950-06-06','90210','LA','Beverly Hills','100 Jetset Blvd');
insert into user values('jackson5','mjackson@gmail.com','1111111','Michael','NULL','Jackson','1958-08-29','90210','LA','Beverly Hills','1050 Musicguy Blvd');
insert into user values('bieber','justinb@gmail.com','66666666','Justin','NULL','Bieber','1994-03-01','90210','LA','Beverly Hills','888 Neverever Blvd');
insert into user values('ross','rossgeller@yahoo.com','ross1111','David','R','Schwimmer','1970-06-20','10026','NY','Manhattan','94 Friends Street');
insert into user values('sia','sia@gmail.com','00001111','Sia','NULL','Lebouf','1983-07-20','90210','LA','Beverly Hills','102 Sohandsome Blvd');
insert into user values('sheldor','sheldonc@gmail.com','81688834','Sheldon','NULL','Cooper','1985-01-23','90210','LA','Beverly Hills','314 Geekyguy Blvd');
insert into user values('amyf','amy@gmail.com','amyffamy','Amy','F','Fowler','1985-05-05','90211','LA','Beverly Hills','815 Smarty Blvd');
insert into user values('bob','marleyb@hotmail.com','3333marl','Bob','M','Marley','1940-01-01','90210','LA','Malibu','111 Sunshine Street');
insert into user values('youknownothing','johnsnow@gmail.com','iknow111','John','K','Snow','1955-01-02','100011','NY','Brooklyn','123 Nobody Blvd');
insert into user values('kelly','kelly@gmail.com','kl818111','Kelly','S','John','1990-11-10','98120','WA','Bellevue','82 Nicecy Blvd');
insert into user values('elleng','deg@gmail.com','ellensm1','Ellen','D','Generes','1962-10-10','90210','WA','Beverly Hills','12 Myshow Blvd');
insert into user values('Obama','care@gmail.com','power123','Barack','H','Obama','1961-08-04','20500','DC','Beverly Hills','1600 Pennsylvania Ave. NW');
insert into user values('queenb','beyonce@gmail.com','0b0b0b0b','Beyonce','R','Carter','1984-11-11','90210','LA','Beverly Hills','1050 Queenb Blvd');
insert into user values('selent','selengt@gmail.com','12341111','Selen','NULL','Taskin','1981-03-07','98121','WA','Seattle','81 Clay Street');
insert into user values('gokayt','gokaytaskin@gmail.com','99999999','Gokay','NULL','Taskin','1982-07-21','98121','WA','Seattle','81 Clay Street');
insert into user values('minc','theinstructor@gmail.com','uwbuwb11','Min','T','Chen','1985-01-02','98011','WA','Bothell','198 Beardslee Blvd');
insert into user values('scott','scottth@hotmail.com','scotty01','Scott','M','Madeup','1982-07-07','98121','WA','Seattle','181 Whatever Street');
insert into user values('penny','pennyme@gmail.com','bigbang1','Penny','M','Brown','1987-07-28','98121','LA','Pasadena','1020 Cheesecake Ave. SW');

-- BOOK
insert into book values('978-0385743563','Hardcover','Steelheart','Brandon Sanderson');
insert into book values('978-0385743570','Paperback','Steelheart','Brandon Sanderson');
insert into book values('978-1423176374','Hardcover','All Our Yesterdays','Cristin Terrill');
insert into book values('978-1616203221','Paperback','Life After Life','Jill McCorkle');
insert into book values('978-0316176491','Paperback','Life After Life','Kate Atkinson');
insert into book values('978-0553293357','Paperback','Foundation','Isaac Asimov');
insert into book values('978-0486411095','Hardcover','Dracula','Bram Stoker');
insert into book values('978-1451645859','Paperback','Ordinary Grace','William Kent Krueger');
insert into book values('978-0312577223','Hardcover','The Nightingale','Kristin Hannah');
insert into book values('978-1616149987','Paperback','The Life We Bury','Allen Eskens');
insert into book values('978-1477820018','Paperback','Take Me With You','Catherine Ryan Hyde');
insert into book values('978-0451419927','Paperback','Secrets of a Charmed Life','Susan Meissner');
insert into book values('978-1250074355','Paperback','The Silent Sister','Diane Chamberlain');
insert into book values('978-0143127550','Paperback','Everything I Never Told You','Celeste Ng');
insert into book values('978-0758278456','Paperback','What She Left Behind','Ellen M. Wiseman');
insert into book values('978-1477822081','Paperback','Trail of Broken Wings','Sejal Badani');
insert into book values('978-1594633669','Hardcover','The Girl on the Train','Paula Hawkins');
insert into book values('978-1507704530','Paperback','The Way We Fall','Cassia Leo');
insert into book values('978-1611099799','Paperback','When I Found You','Catherine Ryan Hyde');
insert into book values('978-0804171472','Paperback','The Narrow Road to the Deep North','Richard Flanagan');
insert into book values('978-1451681758','Paperback','The Light Between Oceans','M.L. Stedman');
insert into book values('978-0143121701','Paperback','The Invention of Wings','Sue Monk Kidd');
insert into book values('978-0812993547','Hardcover','Between the World and Me','Ta-Nehisi Coates');
insert into book values('978-0812988406','Hardcover','When Breath Becomes Air','Deckle Edge');
insert into book values('978-1503944435','Paperback','The Good Neighbor','A. J. Banner');
insert into book values('978-0615590585','Paperback','The Pecan Man','Cassie D. Selleck');
insert into book values('978-0553418026','Paperback','The Martian','Andy Weir');
insert into book values('978-0547577319','Paperback','A Long Walk to Water','Linda Sue Park');
insert into book values('978-0062315007','Paperback','The Alchemist','Paulo Coelho');
insert into book values('978-0451419910','Paperback','A Fall of Marigolds','Susan Meissner');
insert into book values('978-0062413864','Paperback','What She Knew','Gilly Macmillan');
insert into book values('978-1594634475','Hardcover','Fates and Furies','Lauren Groff');
insert into book values('978-1476733951','Paperback','Wool','Hugh Howey');
insert into book values('978-0758278432','Paperback','The Plum Tree','Ellen M. Wiseman');
insert into book values('978-0451466693','Paperback','Flight of the Sparrow','Amy B. Brown');
insert into book values('978-1496400802','Paperback','Secrets She Kept','Cathy Gohlke');
insert into book values('978-0061950728','Paperback','Orphan Train','Christina B. Kline');
insert into book values('978-0425261019','Paperback','Let Us Pretend This Never Happened','Jenny Lawson');
insert into book values('978-1501106477','Paperback','Brooklyn','Colm Toibin');
insert into book values('978-0989866217','Paperback','Miracle Man','William R Leibowitz');
insert into book values('978-0385376716','Hardcover','The Wonderful Things You Will Be','Emily W. Martin');
insert into book values('978-1250095893','Hardcover','Boys in the Trees','Carly Simon');
insert into book values('978-0316380867','Hardcover','The Thing About Jellyfish','Ali Benjamin');
insert into book values('978-0316225885','Hardcover','The Crossing','Michael Connelly');
insert into book values('978-1455581153','Paperback','Mean Streak','Sandra Brown');
insert into book values('978-1476728742','Hardcover','The Wright Brothers','David McCullough');
insert into book values('978-1476761671','Paperback','Before I Go','Colleen Oakley');
insert into book values('978-1476738024','Paperback','A Man Called Ove','Fredrik Backman');
insert into book values('978-1455559817','Paperback','Memory Man','David Baldacci');
insert into book values('978-0425247440','Paperback','What Alice Forgot','Liane Moriarty');
insert into book values('978-1503950252','Paperback','The Short Drop','Matthew FitzSimmons');


-- LISTING
insert into listing values('skyler','978-0385743563','24.99','14');
insert into listing values('testerT','978-1616203221','29.99','3');
insert into listing values('skyler','978-1423176374','19.99','10');
insert into listing values('skyler','978-1503950252','9.99','5');
insert into listing values('skyler','978-0425247440','19.99','5');
insert into listing values('smithg','978-1455559817','24.99','8');
insert into listing values('smithg','978-1476738024','14.99','5');
insert into listing values('AliG','978-1476761671','19.99','4');
insert into listing values('AliG','978-0385743563','22.99','3');
insert into listing values('jamesB','978-1455581153','24.99','5');
insert into listing values('jamesB','978-0316225885','9.99','7');
insert into listing values('jamesB','978-1501106477','14.99','8');
insert into listing values('jackson5','978-0385743563','9.99','20');
insert into listing values('bieber','978-0385743570','19.99','4');
insert into listing values('bieber','978-1423176374','14.99','3');
insert into listing values('bieber','978-1616203221','24.99','2');
insert into listing values('bieber','978-0316176491','19.99','5');
insert into listing values('bieber','978-0553293357','24.99','8');
insert into listing values('bieber','978-0486411095','19.99','9');
insert into listing values('bieber','978-1451645859','9.99','15');
insert into listing values('ross','978-0312577223','26.99','7');
insert into listing values('sia','978-1616149987','18.99','5');
insert into listing values('sia','978-1477820018','19.99','5');
insert into listing values('sia','978-0451419927','16.99','7');
insert into listing values('sheldor','978-1250074355','13.99','9');
insert into listing values('amyf','978-0143127550','19.99','4');
insert into listing values('amyf','978-0062315007','19.99','5');
insert into listing values('amyf','978-0451419910','23.99','7');
insert into listing values('amyf','978-0062413864','19.99','6');
insert into listing values('amyf','978-1594634475','29.99','2');
insert into listing values('amyf','978-1476733951','19.99','4');
insert into listing values('bob','978-0758278456','29.99','3');
insert into listing values('youknownothing','978-1477822081','39.99','1');
insert into listing values('kelly','978-1594633669','29.99','5');
insert into listing values('elleng','978-1507704530','19.99','13');
insert into listing values('elleng','978-0758278432','21.99','10');
insert into listing values('elleng','978-0451466693','9.99','15');
insert into listing values('elleng','978-1496400802','19.99','3');
insert into listing values('Obama','978-1611099799','22.99','5');
insert into listing values('Obama','978-0061950728','23.99','7');
insert into listing values('queenb','978-0804171472','14.99','7');
insert into listing values('queenb','978-0425261019','24.99','4');
insert into listing values('queenb','978-1501106477','12.99','7');
insert into listing values('queenb','978-0989866217','11.99','11');
insert into listing values('queenb','978-0385376716','14.99','5');
insert into listing values('queenb','978-1250095893','12.99','8');
insert into listing values('queenb','978-0316380867','24.99','3');
insert into listing values('selent','978-1451681758','19.99','5');
insert into listing values('selent','978-0316225885','19.99','4');
insert into listing values('selent','978-1455581153','23.99','3');
insert into listing values('gokayt','978-0143121701','13.99','5');
insert into listing values('gokayt','978-0812993547','14.99','12');
insert into listing values('gokayt','978-0812988406','19.99','6');
insert into listing values('gokayt','978-1503944435','12.99','18');
insert into listing values('minc','978-0615590585','22.99','5');
insert into listing values('minc','978-1476728742','14.99','3');
insert into listing values('minc','978-1476761671','17.99','4');
insert into listing values('minc','978-1476738024','13.99','16');
insert into listing values('minc','978-1455559817','19.99','5');
insert into listing values('scott','978-0553418026','15.99','13');
insert into listing values('penny','978-0547577319','12.99','6');
insert into listing values('penny','978-0425247440','14.99','7');
insert into listing values('penny','978-1503950252','11.99','11');


-- USER_REVIEW
insert into user_review (Reviewer, Reviewee, Rating) VALUES('skyler', 'testerT', '4');
insert into user_review (Reviewer, Reviewee, Rating) VALUES('smithg', 'selent', '5');
insert into user_review (Reviewer, Reviewee, Rating) VALUES('Obama', 'bob', '4');
insert into user_review (Reviewer, Reviewee, Rating) VALUES('ross', 'sia', '4');
insert into user_review (Reviewer, Reviewee, Rating) VALUES('amyf', 'selent', '5');
insert into user_review (Reviewer, Reviewee, Rating) VALUES('Obama', 'minc', '4');
insert into user_review (Reviewer, Reviewee, Rating) VALUES('queenb', 'penny', '3');
insert into user_review (Reviewer, Reviewee, Rating) VALUES('scott', 'minc', '4');
insert into user_review (Reviewer, Reviewee, Rating) VALUES('scott', 'sia', '2');
insert into user_review (Reviewer, Reviewee, Rating) VALUES('ross', 'minc', '4');
insert into user_review (Reviewer, Reviewee, Rating) VALUES('skyler', 'bob', '3');
insert into user_review (Reviewer, Reviewee, Rating) VALUES('youknownothing', 'penny', '2');
insert into user_review (Reviewer, Reviewee, Rating) VALUES('skyler', 'minc', '5');
insert into user_review (Reviewer, Reviewee, Rating) VALUES('elleng', 'selent', '4');
insert into user_review (Reviewer, Reviewee, Rating) VALUES('elleng', 'bob', '3');
insert into user_review (Reviewer, Reviewee, Rating) VALUES('elleng', 'penny', '4');
insert into user_review (Reviewer, Reviewee, Rating) VALUES('skyler', 'penny', '4');
insert into user_review (Reviewer, Reviewee, Rating) VALUES('ross', 'penny', '4');
insert into user_review (Reviewer, Reviewee, Rating) VALUES('amyf', 'jackson5', '4');

-- BOOK_REVIEW
insert into book_review values('skyler','978-0385743563','Best book ever, OMG.','5');
insert into book_review values('smithg','978-1503950252','Not very interesting.','3');
insert into book_review values('ross','978-1503950252','Love it','5');
insert into book_review values('skyler','978-1503950252','fascinating','5');
insert into book_review values('Obama','978-0316225885','Interesting subject','4');
insert into book_review values('Obama','978-1507704530','so good','4');
insert into book_review values('ross','978-1496400802','bored','2');
insert into book_review values('elleng','978-0062413864','too confusing','2');
insert into book_review values('elleng','9978-0062315007','very good','5');
insert into book_review values('scott','9978-0062315007','I do not like it at all','1');
insert into book_review values('scott','978-0812988406','not very interesting','3');
insert into book_review values('queenb','978-0812988406','the best book ever','5');
insert into book_review values('queenb','978-1616203221','worst book ever','1');
insert into book_review values('amyf','978-1616203221','amazing book','5');
insert into book_review values('amyf','978-0451419910','nice book','3');
insert into book_review values('smithg','978-1455581153','it is the best book I have ever read','5');
insert into book_review values('AliG','978-1455581153','so boring','2');
insert into book_review values('AliG','978-1476761671','too long to read','1');
insert into book_review values('smithg','978-1476761671','interesting story','3');


-- TRANSACTION
insert into transact values('skyler','testerT','978-1616203221','2016-03-03 12:11:03.232');
insert into transact values('skyler','bieber','978-0385743570','2016-04-03 15:19:04.273');
insert into transact values('skyler','smithg','978-1455559817','2016-02-03 11:22:05.173');
insert into transact values('testerT','sia','978-1616149987','2016-03-03 12:32:06.321');
insert into transact values('testerT','sia','978-1477820018','2016-02-03 15:28:07.123');
insert into transact values('sia','queenb','978-0804171472','2016-05-03 10:43:08.321');
insert into transact values('sia','queenb','978-0425261019','2016-01-03 09:23:09.463');
insert into transact values('sia','queenb','978-1501106477','2016-02-03 14:10:10.123');
insert into transact values('queenb','minc','978-0615590585','2016-03-03 16:11:11.421');
insert into transact values('queenb','minc','978-1476728742','2016-05-03 17:38:12.324');
insert into transact values('queenb','minc','978-1476761671','2016-04-03 18:28:13.421');
insert into transact values('queenb','minc','978-1455559817','2016-03-03 19:33:14.521');
insert into transact values('minc','elleng','978-1507704530','2016-02-03 20:11:15.423');
insert into transact values('minc','elleng','978-0758278432','2016-01-03 21:44:16.143');
insert into transact values('minc','elleng','978-0451466693','2016-28-02 19:32:17.133');
insert into transact values('minc','Obama','978-1611099799','2016-27-02 20:19:18.322');
insert into transact values('Obama','amyf','978-0143127550','2016-26-02 17:20:19.123');
insert into transact values('Obama','amyf','978-0062413864','2016-01-03 15:12:20.432');
insert into transact values('Obama','amyf','978-0553293357','2016-02-03 16:11:21.123');