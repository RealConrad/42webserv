#!/usr/bin/python

import os

query_string = os.environ.get("QUERY_STRING", "")

params = {}
if query_string:
	pairs = query_string.split("&")
	for pair in pairs:
		if '=' in pair:
			key, value = pair.split("=")
			params[key] = value

param_value = params.get("param", "Hello World!")

for i in range(5):
	print(f"<p>{param_value}</p>")