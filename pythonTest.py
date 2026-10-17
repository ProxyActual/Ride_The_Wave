import pandas as pd
import yfinance as yf

# Fetch DOGE-USD data at 5-minute intervals
df = yf.download("DOGE-USD", period="59d", interval="5m")

# Collapse the ticker level so only a single header row remains
if isinstance(df.columns, pd.MultiIndex):
    df.columns = df.columns.get_level_values(0)
df.columns.name = None

# Convert the timestamp index to Unix time (seconds)
timestamps = pd.to_datetime(df.index, utc=True)
df.index = timestamps.as_unit("s").asi8

df.to_csv("doge_5m_yahoo.csv", index_label="Date")

print("Success! Generated 'doge_5m_yahoo.csv' using Yahoo Finance.")
