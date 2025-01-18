-- Создание таблицы temperature
CREATE TABLE temperature (
    timestamp BIGINT NOT NULL,  -- Поле для времени UNIXTIME
    value FLOAT NOT NULL,         -- Поле для значения температуры
    PRIMARY KEY (timestamp)      -- Уникальный индекс по timestamp
);