#!/usr/bin/env python

import os

post_data  = os.getenv('POST_DATA', '')
post_value = post_data.split('=')[-1]

if not post_data:
    print("Content-Type: text/html")
    print("")
    print("<html>")
    print("<head>")
    print("<title>CGI Script</title>")
    print("<link rel=\"stylesheet\" type=\"text/css\" href=\"/cgi-bin/css/post.css\">")
    print("</head>")
    print("<body>")
    print("<div class=\"container\">")
    print("<img src=\"/cgi-bin/img/indicating.jpg\" alt=\"hand\" class=\"hand left-hand\">")
    print("<div class=\"centered-text\">")
    print("<h1>CGI Script Output</h1>")
    print("<p>POST data:</p>")
    print("<h2>Your content is empty</h2>")
    print("</div>")
    print("<img src=\"/cgi-bin/img/indicating.jpg\" alt=\"hand\" class=\"hand right-hand\">")
    print("</div>")
    print("</body>")
else:
    post_value = post_data.split('=')[-1]
    print("Content-Type: text/html")
    print("")
    print("<html>")
    print("<head>")
    print("<title>CGI Script</title>")
    print("<link rel=\"stylesheet\" type=\"text/css\" href=\"/cgi-bin/css/post.css\">")
    print("</head>")
    print("<body>")
    print("<div class=\"container\">")
    print("<img src=\"/cgi-bin/img/indicating.jpg\" alt=\"hand\" class=\"hand left-hand\">")
    print("<div class=\"centered-text\">")
    print("<h1>CGI Script Output</h1>")
    print("<p>POST data:</p>")
    print("<pre>{}</pre>".format(post_value))
    print("</div>")
    print("<img src=\"/cgi-bin/img/indicating.jpg\" alt=\"hand\" class=\"hand right-hand\">")
    print("</div>")
    print("</body>")
    print("</html>")