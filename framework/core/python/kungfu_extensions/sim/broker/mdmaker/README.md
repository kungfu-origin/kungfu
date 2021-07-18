# mdmaker: Market Data Generator

This is a set of python scripts that create random market data, useful for testing.


## Usage

`stock.py` generates a csv with the top of book on each line with a valid timestamp.

`merge.py` merges two top-of-book csvs using the timestamps for the operations.


## Dependencies

```
pip3 install sortedcollections
pip3 install recordclass

```
