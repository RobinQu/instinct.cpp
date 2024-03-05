import unittest
from langchain.text_splitter import RecursiveCharacterTextSplitter, CharacterTextSplitter


class TextSplitter(unittest.TestCase):
    def test_something(self):
        # ts = RecursiveCharacterTextSplitter(chunk_size=20, chunk_overlap=3)
        ts = CharacterTextSplitter(separator=" ", chunk_size=7, chunk_overlap=3)
        result = ts.split_text("foo bar baz 123")
        print(result)


if __name__ == '__main__':
    unittest.main()
