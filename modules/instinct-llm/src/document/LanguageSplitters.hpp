//
// Created by RobinQu on 2024/3/2.
//

#ifndef LANGUAGECHARACTERTEXTSPLITTERS_HPP
#define LANGUAGECHARACTERTEXTSPLITTERS_HPP
#include <vector>
#include <unordered_map>
#include <string>
#include <unicode/ustring.h>
#include "CoreGlobals.hpp"


namespace INSTINCT_LLM_NS {
    /**
     * COPIED FROM LANGCHIAN'S text_splitter.py
     */
    namespace seperators {
        using SEPERATORS = std::vector<U_ICU_NAMESPACE::UnicodeString>;

        enum Language {
            UNKNOWN,
            CPP,
            GO,
            JAVA,
            KOTLIN,
            JS,
            TS,
            PHP,
            PROTO,
            PYTHON,
            RST,
            RUBY,
            RUST,
            SCALA,
            SWIFT,
            MARKDOWN,
            LATEX,
            HTML,
            CSHARP,
            SOL,
            COBOL
        };

        static std::unordered_map<Language, SEPERATORS> SEPERATORS_FOR_LANGUAGE = {
            {
                CPP, {

                    "\nclass ",

                    "\nvoid ",
                    "\nint ",
                    "\nfloat ",
                    "\ndouble ",

                    "\nif ",
                    "\nfor ",
                    "\nwhile ",
                    "\nswitch ",
                    "\ncase ",

                    "\n\n",
                    "\n",
                    " ",
                    "",
                }
            },
            {
                GO, {

                    "\nfunc ",
                    "\nvar ",
                    "\nconst ",
                    "\ntype ",

                    "\nif ",
                    "\nfor ",
                    "\nswitch ",
                    "\ncase ",

                    "\n\n",
                    "\n",
                    " ",
                    "",
                }
            },
            {
                JAVA, {

                    "\nclass ",

                    "\npublic ",
                    "\nprotected ",
                    "\nprivate ",
                    "\nstatic ",

                    "\nif ",
                    "\nfor ",
                    "\nwhile ",
                    "\nswitch ",
                    "\ncase ",

                    "\n\n",
                    "\n",
                    " ",
                    "",
                }
            },
            {
                KOTLIN, {

                    "\nclass ",

                    "\npublic ",
                    "\nprotected ",
                    "\nprivate ",
                    "\ninternal ",
                    "\ncompanion ",
                    "\nfun ",
                    "\nval ",
                    "\nvar ",

                    "\nif ",
                    "\nfor ",
                    "\nwhile ",
                    "\nwhen ",
                    "\ncase ",
                    "\nelse ",

                    "\n\n",
                    "\n",
                    " ",
                    "",
                }
            },
            {
                JS, {

                    "\nfunction ",
                    "\nconst ",
                    "\nlet ",
                    "\nvar ",
                    "\nclass ",

                    "\nif ",
                    "\nfor ",
                    "\nwhile ",
                    "\nswitch ",
                    "\ncase ",
                    "\ndefault ",

                    "\n\n",
                    "\n",
                    " ",
                    "",
                }
            },
            {
                TS, {
                    "\nenum ",
                    "\ninterface ",
                    "\nnamespace ",
                    "\ntype ",

                    "\nclass ",

                    "\nfunction ",
                    "\nconst ",
                    "\nlet ",
                    "\nvar ",

                    "\nif ",
                    "\nfor ",
                    "\nwhile ",
                    "\nswitch ",
                    "\ncase ",
                    "\ndefault ",

                    "\n\n",
                    "\n",
                    " ",
                    "",
                }
            },
            {
                PHP, {

                    "\nfunction ",

                    "\nclass ",

                    "\nif ",
                    "\nforeach ",
                    "\nwhile ",
                    "\ndo ",
                    "\nswitch ",
                    "\ncase ",

                    "\n\n",
                    "\n",
                    " ",
                    "",
                }
            },
            {
                PROTO, {

                    "\nmessage ",

                    "\nservice ",

                    "\nenum ",

                    "\noption ",

                    "\nimport ",

                    "\nsyntax ",

                    "\n\n",
                    "\n",
                    " ",
                    "",
                }
            },
            {
                PYTHON, {
                    "\nclass ",
                    "\ndef ",
                    "\n\tdef ",

                    "\n\n",
                    "\n",
                    " ",
                    "",
                }
            },
            {
                RST, {
                    "\n=+\n",
                    "\n-+\n",
                    "\n\\*+\n",

                    "\n\n.. *\n\n",

                    "\n\n",
                    "\n",
                    " ",
                    "",
                }
            },
            {
                RUBY, {

                    "\ndef ",
                    "\nclass ",

                    "\nif ",
                    "\nunless ",
                    "\nwhile ",
                    "\nfor ",
                    "\ndo ",
                    "\nbegin ",
                    "\nrescue ",

                    "\n\n",
                    "\n",
                    " ",
                    "",
                }
            },
            {
                RUST, {

                    "\nfn ",
                    "\nconst ",
                    "\nlet ",

                    "\nif ",
                    "\nwhile ",
                    "\nfor ",
                    "\nloop ",
                    "\nmatch ",
                    "\nconst ",

                    "\n\n",
                    "\n",
                    " ",
                    "",
                }
            },
            {
                SCALA, {
                    "\nclass ",
                    "\nobject ",

                    "\ndef ",
                    "\nval ",
                    "\nvar ",

                    "\nif ",
                    "\nfor ",
                    "\nwhile ",
                    "\nmatch ",
                    "\ncase ",

                    "\n\n",
                    "\n",
                    " ",
                    "",
                }
            },
            {
                SWIFT, {

                    "\nfunc ",

                    "\nclass ",
                    "\nstruct ",
                    "\nenum ",

                    "\nif ",
                    "\nfor ",
                    "\nwhile ",
                    "\ndo ",
                    "\nswitch ",
                    "\ncase ",

                    "\n\n",
                    "\n",
                    " ",
                    "",
                }
            },
            {
                MARKDOWN, {

                    "\n#{1,6} ",
                    "```\n",
                    "\n\\*\\*\\*+\n",
                    "\n---+\n",
                    "\n___+\n",
                    "\n\n",
                    "\n",
                    " ",
                    "",
                }
            },
            {
                LATEX, {
                    "\n\\\\chapter{",
                    "\n\\\\section{",
                    "\n\\\\subsection{",
                    "\n\\\\subsubsection{",

                    "\n\\\\begin{enumerate}",
                    "\n\\\\begin{itemize}",
                    "\n\\\\begin{description}",
                    "\n\\\\begin{list}",
                    "\n\\\\begin{quote}",
                    "\n\\\\begin{quotation}",
                    "\n\\\\begin{verse}",
                    "\n\\\\begin{verbatim}",

                    "\n\\\begin{align}",
                    "$$",
                    "$",

                    " ",
                    "",
                }
            },
            {
                HTML, {
                    "<body",
                    "<div",
                    "<p",
                    "<br",
                    "<li",
                    "<h1",
                    "<h2",
                    "<h3",
                    "<h4",
                    "<h5",
                    "<h6",
                    "<span",
                    "<table",
                    "<tr",
                    "<td",
                    "<th",
                    "<ul",
                    "<ol",
                    "<header",
                    "<footer",
                    "<nav",

                    "<head",
                    "<style",
                    "<script",
                    "<meta",
                    "<title",
                    "",
                }
            },
            {
                CSHARP, {
                    "\ninterface ",
                    "\nenum ",
                    "\nimplements ",
                    "\ndelegate ",
                    "\nevent ",

                    "\nclass ",
                    "\nabstract ",

                    "\npublic ",
                    "\nprotected ",
                    "\nprivate ",
                    "\nstatic ",
                    "\nreturn ",

                    "\nif ",
                    "\ncontinue ",
                    "\nfor ",
                    "\nforeach ",
                    "\nwhile ",
                    "\nswitch ",
                    "\nbreak ",
                    "\ncase ",
                    "\nelse ",

                    "\ntry ",
                    "\nthrow ",
                    "\nfinally ",
                    "\ncatch ",

                    "\n\n",
                    "\n",
                    " ",
                    "",
                }
            },
            {
                SOL, {

                    "\npragma ",
                    "\nusing ",
                    "\ncontract ",
                    "\ninterface ",
                    "\nlibrary ",
                    "\nconstructor ",
                    "\ntype ",
                    "\nfunction ",
                    "\nevent ",
                    "\nmodifier ",
                    "\nerror ",
                    "\nstruct ",
                    "\nenum ",
                    "\nif ",
                    "\nfor ",
                    "\nwhile ",
                    "\ndo while ",
                    "\nassembly ",
                    "\n\n",
                    "\n",
                    " ",
                    "",
                }
            },
            {
                COBOL, {
                    "\nIDENTIFICATION DIVISION.",
                    "\nENVIRONMENT DIVISION.",
                    "\nDATA DIVISION.",
                    "\nPROCEDURE DIVISION.",

                    "\nWORKING-STORAGE SECTION.",
                    "\nLINKAGE SECTION.",
                    "\nFILE SECTION.",
                    "\nINPUT-OUTPUT SECTION.",
                    "\nOPEN ",
                    "\nCLOSE ",
                    "\nREAD ",
                    "\nWRITE ",
                    "\nIF ",
                    "\nELSE ",
                    "\nMOVE ",
                    "\nPERFORM ",
                    "\nUNTIL ",
                    "\nVARYING ",
                    "\nACCEPT ",
                    "\nDISPLAY ",
                    "\nSTOP RUN.",
                    "\n",
                    " ",
                    "",
                }
            }

        };
    }
}

#endif //LANGUAGECHARACTERTEXTSPLITTERS_HPP
