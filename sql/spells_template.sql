Drop view spells_template;


CREATE VIEW spells_template AS
select s.entry, s.name, s.sp, s.element, s.hit, s.LearnPoint, s.LevelMax,
       s.type, s.DamageMod, s.reborn, s.Core
  from spell_template s
 union
select i.modelid AS entry, i.name, 0 AS sp, 0 AS element, 1 AS hit,
       0 AS LearnPoint, 1 AS LevelMax, 3 AS type, 6 AS DamageMod,
       0 AS reborn, 0 AS Core
  from item_template i
 where ((i.type = 'Food') or (i.type = 'Medicine') or (i.type = 'Power') or
        (i.type = 'RevivePill') or (i.type = 'Leaf') or (i.type = 'Scroll'));
