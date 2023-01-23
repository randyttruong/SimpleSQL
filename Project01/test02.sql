-- The following is a select statement:
select *          -- * means all columns
from Movies       -- from the Movies table
where ID = 123    -- filter
order by Title asc  -- order by => sort
limit 10            -- ignore this 3.14159
-- no $ at the end, you should recognize EOF as EOS
