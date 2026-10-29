import argparse
import shutil
from pathlib import Path

import pandas as pd
import yfinance as yf


def parse_args():
    parser = argparse.ArgumentParser(description="Download Yahoo Finance price history to CSV.")
    parser.add_argument("--symbol", default="DOGE-USD", help="Yahoo ticker, e.g. DOGE-USD")
    parser.add_argument("--period", default="59d", help="History length, e.g. 59d or 2y")
    parser.add_argument("--interval", default="5m", help="Candle size, e.g. 5m or 1d")
    parser.add_argument("--output", default="doge_5m_yahoo.csv", help="CSV file to write")
    parser.add_argument("--no-copy", action="store_true",
                        help="Do not copy the output into the 'histories' directory")
    return parser.parse_args()


def main():
    args = parse_args()

    df = yf.download(args.symbol, period=args.period, interval=args.interval,
                     progress=False, auto_adjust=False)
    if df is None or df.empty:
        raise SystemExit(f"No data returned for {args.symbol}")

    # Collapse the ticker level so only a single header row remains
    if isinstance(df.columns, pd.MultiIndex):
        df.columns = df.columns.get_level_values(0)
    df.columns.name = None
    df = df[[c for c in ["Close", "High", "Low", "Open", "Volume"] if c in df.columns]]

    # Percent change of the close from one row (day) to the next
    df["PercentChange"] = df["Close"].pct_change() * 100.0
    df = df.dropna(subset=["PercentChange"])

    # Convert the timestamp index to Unix time (seconds)
    timestamps = pd.to_datetime(df.index, utc=True)
    df.index = timestamps.as_unit("s").asi8

    output = Path(args.output)
    output.parent.mkdir(parents=True, exist_ok=True)
    df.to_csv(output, index_label="Date")

    if not args.no_copy:
        histories_dir = Path("histories")
        histories_dir.mkdir(exist_ok=True)
        if output.resolve().parent != histories_dir.resolve():
            shutil.copy2(output, histories_dir / output.name)

    print(f"Success! Generated '{output}' with {len(df)} rows.")


if __name__ == "__main__":
    main()
