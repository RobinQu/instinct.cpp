import unittest
import re
from typing import List



class RegexTest(unittest.TestCase):
    def test_split_with_group(self):
        seperators = ["\n\n", "\n", " ", ""]
        text = "Note that, in this example, words is a local, or stack array of actual UnicodeString objects. No heap allocation is involved in initializing this array of empty strings (C++ is not Java!). Local UnicodeString arrays like this are a very good fit for use with split(); after extracting the fields, any values that need to be kept in some more permanent way can be copied to their ultimate destination."
        for separator in seperators:
            # splits = re.split(f"({separator})", text)
            splits = re.split(text, separator)
            print(splits)



if __name__ == '__main__':
    unittest.main()
