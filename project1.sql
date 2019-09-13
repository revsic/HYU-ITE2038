-- 1. Print name of grass type animal in dictionary order.
select name from Pokemon where type = 'Grass' order by name;

-- 2. Print name of the trainer from Brown city or Rainbow city in dictionary order.
select name from Trainer
where hometown = 'Brown City' or hometown = 'Rainbow City' order by name;

-- 3. Print types of pokemon in dictionary order.
select distinct type from Pokemon order by type;

-- 4. Print the name of city which starts with 'B' in dictionary order.
select name from City where name like 'B%' order by name;

-- 5. Print the hometowns of trainer whose name doesn't start with 'M' in dictionary order.
select distinct hometown from Trainer where not name like 'M%' order by hometown;

-- 6. Print the nickname of caught pokemon which has maximum level among them.
select nickname from CatchedPokemon
where level = (select max(level) from CatchedPokemon)
order by nickname;

-- 7. Print the name of the pokemon which starts with vowel in dictionary order. 
select name from Pokemon
where
  name like 'A%' or
  name like 'E%' or
  name like 'I%' or
  name like 'O%' or
  name like 'U%'
order by name;

-- 8. Print the average level of the caught pokemons.
select avg(level) from CatchedPokemon;

-- 9. Print the maximum level of the pokemon which was caught by Yellow.
select max(level) from CatchedPokemon
where owner_id in (select id from Trainer where name = 'Yellow');

-- 10. Print the hometowns of trainers in dictionary order.
select distinct hometown from Trainer order by hometown;

-- 11. Print the nickname of pokemon which starts with 'A' 
-- and name of the trainer who caught them in dictionary order of the name of trainer.
select t.name, c.nickname from Trainer t, CatchedPokemon c
where t.id = c.owner_id and c.nickname like 'A%' order by t.name;

-- 12. Print the name of the leader of city which has property of Amazon.
select name from Trainer where id in (
  select leader_id from Gym where city in (
    select name from City where description = 'Amazon'));

-- 13. Print the id of trainer who caught the maximum number of fire type pokemon
-- and the number of it.
select owner_id, count(*) as cnt from CatchedPokemon
where pid in (select id from Pokemon where type = 'Fire')
group by owner_id order by cnt desc limit 1;

-- 14. Print the type of pokemon which has single digit as id in descending order.
select distinct type from Pokemon where id < 10 order by id desc;

-- 15. Print the number of pokemon which doesn't have fire type.
select count(*) from Pokemon where type != 'Fire';

-- 16. Print the name of pokemon which has bigger id before evolution in dictionary order.
select name from Pokemon
where id in (select before_id from Evolution where before_id > after_id)
order by name;

-- 17. Print the average level of caught water-type pokemon.
select avg(level) from CatchedPokemon
where pid in (select id from Pokemon where type = 'Water');

-- 18. Print the nickname of caught pokemon
-- which is caught by the leader of gym and has maximum level among them.
select nickname from CatchedPokemon
where level = (
  select max(level) from CatchedPokemon
  where owner_id in (select leader_id from Gym));

-- 19. Print the name of the trainer
-- who is from Blue city and has highest average level of caught pokemon.
select name from Trainer
where id in (
  select c.owner_id
  from (
    select owner_id, avg(level) as avg_level from CatchedPokemon
    where owner_id in (select id from Trainer where hometown = 'Blue City')
    group by owner_id) c
  where c.avg_level = (
    select max(tmp.avg_level) from (
      select avg(level) as avg_level from CatchedPokemon
      where owner_id in (select id from Trainer where hometown = 'Blue City')
      group by owner_id) as tmp))
order by name;

-- 20. Print the name of pokemon
-- which is evolutionable, has electric type and is caught by trainer who doesn't have hometown friend.
select distinct p.name from CatchedPokemon c, Pokemon p
where
  c.pid = p.id
  and c.owner_id in (
    select t.id
    from
      Trainer t,
      (select hometown, count(*) as cnt from Trainer group by hometown) c
    where t.hometown = c.hometown and c.cnt = 1)
  and p.type = 'Electric'
  and c.pid in (select before_id from Evolution);

-- 21. Print the name of the leaders and sum of the level of caught pokemons
-- in desceding order of sum.
select t.name, sum(c.level) as sum_level from CatchedPokemon c, Trainer t
where c.owner_id = t.id and c.owner_id in (select leader_id from Gym)
group by c.owner_id
order by sum_level desc;

-- 22. Print the hometown which has maximum number of trainers.
select hometown, count(*) as cnt from Trainer
group by hometown order by cnt desc limit 1;

-- 23. Print the name of pokemon which is caught by trainers from Sangnok city and brown city
-- in dictionary order.
select name from Pokemon
where id in (
  select distinct pid from CatchedPokemon
  where owner_id in (select id from Trainer where hometown = 'Sangnok City')
    and pid in (select c.pid from CatchedPokemon c, Trainer t
                where c.owner_id = t.id and t.hometown = 'Brown City'))
order by name;

-- 24. Print the name of trainer
-- who is from Sangnok city and caught the pokemon which name starts with P in dict order.
select distinct t.name from CatchedPokemon c, Trainer t
where
  c.owner_id = t.id
  and c.pid in (select id from Pokemon where name like 'P%')
  and t.hometown = 'Sangnok City'
order by t.name;

-- 25. Print the name of trainer and caught pokemons in dictionary order,
-- trainer name as first, pokemon name as second.
select t.name, p.name from Trainer t, CatchedPokemon c, Pokemon p
where
  t.id = c.owner_id
  and c.pid = p.id
order by t.name, p.name;

 -- 26. Print the name of pokemon which is double-evolvable in dictionary order.
select p.name from Evolution e, Pokemon p
where 
  e.before_id = p.id
  and e.after_id in (select before_id from Evolution)
order by p.name;

-- 27. Print the nickname of pokemon which is caught by the leader of Sangnok city
-- and has water-type in dictionary order.
select nickname from CatchedPokemon
where owner_id = (select leader_id from Gym where city = 'Sangnok City')
  and pid in (select id from Pokemon where type = 'Water')
order by nickname;

-- 28. Print the name of trainer who has three or more of evolved pokemons in dict order.
select t.name
from
  Trainer t,
  (select owner_id, count(*) as cnt from CatchedPokemon
  where pid in (select after_id from Evolution)
  group by owner_id) c
where t.id = c.owner_id and c.cnt >= 3
order by t.name;

-- 29. Print the name of pokemon which was not caught by player until now in dictionary order.
select name from Pokemon
where id not in (select distinct pid from CatchedPokemon)
order by name;

-- 30. Print the maximum level of pokemons grouped by hometown in descending order.
select max(c.level) as max_level from CatchedPokemon c, Trainer t
where c.owner_id = t.id
group by t.hometown
order by max_level desc;

-- 31. Print the triple-evolvable pokemons in id order.
select
  (select name from Pokemon where id = e.before_id) as first,
  (select name from Pokemon where id = e.after_id) as second,
  (select name from Pokemon where id = 
    (select after_id from Evolution ie where e.after_id = ie.before_id)) as third
from Evolution e
where e.after_id in (select distinct before_id from Evolution)
order by e.before_id;
