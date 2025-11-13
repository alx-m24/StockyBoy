# 5PercentRule

## Concept

The library implements a **5% rule** for paper trading:

1. For each tracked stock:
   - If the price has dropped **5% or more** over the past *N* days, buy **$5** of it.
   - If the price has increased **5% or more** over the past *N* days and we currently hold some, sell **$5** worth.
2. The library tracks past trades and can eliminate underperforming stocks, replacing them with new symbols for experimentation. Performance-based filtering is optional for now.

This system is designed for **paper trading accounts** only and serves as a simple test for the `StockyBoy` Stock Scraper/Interface.

---

## Usage

1. **Initialize the Library**
   - Provide a list of stock symbols to track.

2. **Run the 5% Rule**
   - Each time the external program (`interface.exe`) launches, the library will:
     - Fetch the past *N* days of stock data for all symbols.
     - Apply the 5% down/up rule for buying or selling $5 increments.
     - Optionally mark stocks for replacement based on trade history.

3. **Paper Trading**
   - All trades are executed on a paper trading account.
   - Trade amounts are intentionally small to allow safe experimentation.

---

## Notes

- This library is for testing and learning trading strategies; it is **not intended for production trading**.
- Trades occur **once per launch** (or manually triggered), not continuously.
- Stock performance tracking and elimination is experimental; the main goal is to validate the workflow and framework for more advanced algorithms in the future.

## Note to Self — Implementation Steps

1. Get a list of stock labels (symbols) to track.
2. Choose an initial subset of **50–75%** of the list to run the algorithm on.
3. Run the 5% rule **daily**.
4. Store a **decay or cooldown** for stocks that were dropped or traded recently to avoid reusing them immediately.
5. Optionally, track the performance of past trades and replace underperforming stocks with new symbols over time.