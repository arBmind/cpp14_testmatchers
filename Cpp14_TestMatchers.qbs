import qbs

Project {
    minimumQbsVersion: "1.6"

    CppApplication {
        consoleApplication: true
        files: [
            ".editorconfig",
            "assert_that.h",
            "expect.h",
            "main.cpp",
            "str_concat.h",
            "str_represent.h",
            "str_template.h",
        ]

        cpp.cxxLanguageVersion: "c++14"
    }
}
