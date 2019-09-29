#1_1
SELECT
  name
FROM
  Pokemon
WHERE
  type = 'Grass'
ORDER BY
  name;



#1_2
SELECT
  name
FROM
  Trainer
WHERE
  hometown = 'Brown City' OR hometown = 'Rainbow City'
ORDER BY
  name;



#1_3
SELECT
  DISTINCT type
FROM
  Pokemon
ORDER BY
  type;



#1_4
SELECT
  name
FROM
  City
WHERE
  name LIKE 'B%'
ORDER BY
  name;



#1_5
SELECT
  hometown
FROM
  Trainer
WHERE
  name NOT LIKE 'M%'
ORDER BY
  hometown;



#1_6
SELECT
  nickname
FROM
  CatchedPokemon cp
WHERE
  cp.level =(
    SELECT
      MAX(level)
    FROM
      CatchedPokemon
  )
ORDER BY
  nickname;



#1_7
SELECT
  name
FROM
  Pokemon
WHERE
  name LIKE 'A%'
  OR name LIKE 'E%'
  OR name LIKE 'I%'
  OR name LIKE 'O%'
  OR name LIKE 'U%';



#1_8
SELECT
  AVG(level)
FROM
  CatchedPokemon;



#1_9
SELECT
  MAX(level)
FROM
  CatchedPokemon cp
  JOIN (
    SELECT
      id
    FROM
      Trainer
    WHERE
      name = 'Yellow'
  ) t ON t.id = cp.owner_id;



#1_10
SELECT
  DISTINCT hometown
FROM
  Trainer
ORDER BY
  hometown;



#1_11
SELECT
  t.name,
  cp.nickname
FROM
  Trainer t
  JOIN CatchedPokemon cp ON cp.owner_id = t.id
WHERE
  cp.nickname LIKE 'A%'
ORDER BY
  t.name;



#1_12
SELECT
  t.name
FROM
  Trainer t,
  (
    SELECT
      name
    FROM
      City
    WHERE
      description = 'Amazon'
  ) c, Gym g
WHERE
  c.name = t.hometown
  AND g.leader_id = t.id;



#1_13
SELECT
  t.id tid, cnt
FROM
  (
    SELECT
      t.id, COUNT(*) cnt
    FROM
      Trainer t
      JOIN CatchedPokemon cp ON t.id = cp.owner_id
      JOIN (
        SELECT
          id
        FROM
          Pokemon
        WHERE
          type = 'Fire'
      ) p ON cp.pid = p.id
    GROUP BY
      t.id
  ) t
WHERE
  cnt = (
    SELECT
      MAX(cnt)
    FROM
      (
        SELECT
          t.id, COUNT(*) cnt
        FROM
          Trainer t
          JOIN CatchedPokemon cp ON t.id = cp.owner_id
          JOIN (
            SELECT
              id
            FROM
              Pokemon
            WHERE
              type = 'Fire'
          )p ON cp.pid = p.id
        GROUP BY
          t.id
      ) t
  );



#2_1
SELECT
  DISTINCT type
FROM
  Pokemon
WHERE
  id < 10
ORDER BY
  id DESC;



#2_2
SELECT
  type, COUNT(type) NotFire
FROM
  Pokemon
WHERE
  type <> 'Fire'
GROUP BY type;



#2_3
SELECT
  p.name
FROM
  Pokemon p
  JOIN Evolution e ON p.id = e.before_id
WHERE
  e.before_id > e.after_id
ORDER BY
  p.name;



#2_4
SELECT
  AVG(level)
FROM
  CatchedPokemon cp
  JOIN Pokemon p ON cp.pid = p.id
WHERE
  p.type = 'Water';



#2_5
SELECT
  cp.nickname
FROM
  CatchedPokemon cp
  JOIN Gym g ON cp.owner_id = g.leader_id
WHERE
  cp.level = (
    SELECT
      MAX(cp.level)
    FROM
      CatchedPokemon cp
      JOIN Gym g ON cp.owner_id = g.leader_id
  );



#2_6
SELECT
  t.name
FROM (
  SELECT
    id, name
  FROM
    Trainer
  WHERE
    hometown = 'Blue city'
  ) t
  JOIN CatchedPokemon cp ON t.id = cp.owner_id
  JOIN (
    SELECT
      t.name,
      AVG(cp.level) avg_level
    FROM (
      SELECT
        id, name
      FROM
        Trainer
      WHERE
        hometown = 'Blue city'
    ) t
      JOIN CatchedPokemon cp ON t.id = cp.owner_id
    GROUP BY t.name
  ) avg ON t.name = avg.name
WHERE
  avg.avg_level = (
    SELECT
      MAX(avg_level)
    FROM (
      SELECT
        AVG(cp.level) avg_level
      FROM (
        SELECT
          id, name
        FROM
          Trainer
        WHERE
          hometown = 'Blue city'
      ) t
        JOIN CatchedPokemon cp ON t.id = cp.owner_id
      GROUP BY t.name
    ) avg
  )
GROUP BY
  t.name
ORDER BY
  t.name;



#2_7
SELECT
  p.name
FROM (
  SELECT
    id, name
  FROM
    Pokemon
  WHERE
    type = 'Electric'
  ) p
  JOIN Evolution yee ON p.id = yee.before_id
  JOIN CatchedPokemon cp ON p.id = cp.pid
  JOIN (
    SELECT
      id
    FROM
      Trainer
    GROUP BY
      hometown
    HAVING COUNT(*) = 1
  ) t ON t.id = cp.owner_id;



#2_8
SELECT
  t.name, SUM(cp.level) lv_sum
FROM
  Trainer t
  JOIN Gym g ON t.id = g.leader_id
  JOIN CatchedPokemon cp ON cp.owner_id = t.id
GROUP BY
  name
ORDER BY
  lv_sum DESC;



#2_9
SELECT
  hometown
FROM (
  SELECT
    hometown, COUNT(*) cnt
  FROM
    Trainer
  GROUP BY
    hometown
  ) t
WHERE
  cnt = (
    SELECT
      MAX(cnt)
    FROM (
      SELECT
        hometown, COUNT(*) cnt
      FROM
        Trainer
      GROUP BY
        hometown
      ) m
    );



#2_10
SELECT
  DISTINCT p.name
FROM
  Pokemon p
WHERE
  p.name IN (
    SELECT
      p.name
    FROM
      Pokemon p
      JOIN CatchedPokemon cp ON p.id=cp.pid
      JOIN (
        SELECT
          id, hometown
        FROM
          Trainer
        WHERE
          hometown='Sangnok City'
      ) t ON cp.owner_id=t.id
  )
  AND p.name IN (
    SELECT
      p.name
    FROM
      Pokemon p
      JOIN CatchedPokemon cp ON p.id=cp.pid
      JOIN (
        SELECT
          id, hometown
        FROM
          Trainer
        WHERE
          hometown='Brown City'
      ) t ON cp.owner_id=t.id
  )
ORDER BY
  p.name;



#3_1
SELECT
  t.name
FROM
  (
    SELECT
      id, name
    FROM
      Pokemon
    WHERE
      name LIKE 'P%'
  ) p
  JOIN CatchedPokemon cp ON p.id=cp.pid
  JOIN Trainer t ON t.id=cp.owner_id
  AND hometown='Sangnok City'
ORDER BY
  name;



#3_2
SELECT
  t.name, p.name
FROM
  Trainer t
  JOIN CatchedPokemon cp ON t.id=cp.owner_id
  JOIN Pokemon p ON p.id=cp.pid
ORDER BY
  t.name, p.name;



#3_3
SELECT
  p.name
FROM
  Pokemon p
  JOIN Evolution e ON p.id=e.before_id
  WHERE e.after_id NOT IN (SELECT before_id FROM Evolution)
  AND e.before_id NOT IN (SELECT after_id FROM Evolution);



#3_4
SELECT
  cp.nickname
FROM
  CatchedPokemon cp
  JOIN (
    SELECT
      leader_id
    FROM
      Gym
    WHERE
      city='Sangnok City'
  ) g ON g.leader_id=cp.owner_id
  JOIN (
    SELECT
      id, type
    FROM
      Pokemon
    WHERE
      type='Water'
  ) p ON p.id=cp.pid
ORDER BY
  cp.nickname;



#3_5
SELECT
  t.name
FROM
  Trainer t
  JOIN (
    SELECT
      cp.owner_id, COUNT(cp.owner_id) cnt
    FROM
      CatchedPokemon cp
      JOIN Evolution e ON e.after_id = cp.pid
    GROUP BY
      cp.owner_id
  ) dcp ON dcp.owner_id=t.id
WHERE
  cnt > 2
ORDER BY
  t.name;



#3_6
SELECT
  p.name
FROM
  Pokemon p
WHERE
  p.id NOT IN (
    SELECT
      p.id
    FROM
      Pokemon p
      JOIN CatchedPokemon cp ON p.id = cp.pid
  )
ORDER BY
  p.name;



#3_7
SELECT
  c.name, MAX(level)
FROM
  City c
  JOIN Trainer t ON c.name = t.hometown
  JOIN CatchedPokemon cp ON t.id = cp.owner_id
GROUP BY
  c.name
ORDER BY
  MAX(level) DESC;



#3_8
SELECT
  p1.id, p1.name, p2.name, p3.name
FROM
  Pokemon p1
  JOIN Evolution e1 ON p1.id = e1.before_id
  JOIN Evolution e2 ON e1.after_id = e2.before_id
  JOIN Pokemon p2 ON p2.id = e2.before_id
  JOIN Pokemon p3 ON e2.after_id = p3.id
WHERE
  e1.after_id IN (
    SELECT
      before_id
    FROM
      Evolution
  )
ORDER BY
  p1.id;
