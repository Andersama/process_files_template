// process_files.cpp : Defines the entry point for the application.
//

#include "process_files.h"
#include "file-cpp/file.h"
#include "utf8-cpp/source/utf8.h"

using namespace std;

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

#include <fstream>
#include <sstream>

//<specifiers> <typename> <qualifiers>
std::string slurp(const char* filename)
{
    std::ifstream in;
    in.open(filename, std::ifstream::in | std::ifstream::binary);
    std::stringstream sstr;
    sstr << in.rdbuf();
    in.close();
    return sstr.str();
}

void slurp(const std::filesystem::path& filepath, std::string& buffer)
{
	std::ifstream in;
	in.open(filepath, std::ifstream::in | std::ifstream::binary);
	buffer.clear();
	buffer.assign((std::istreambuf_iterator<char>(in)), (std::istreambuf_iterator<char>()));
	in.close();
}

void slurp(const std::filesystem::path& filepath, std::vector<char>& buffer)
{
	std::ifstream in;
	in.open(filepath, std::ifstream::in | std::ifstream::binary);
	buffer.clear();
	buffer.assign((std::istreambuf_iterator<char>(in)), (std::istreambuf_iterator<char>()));
	in.close();
}

void string_from(std::string& buf, const std::filesystem::path& path)
{
	// buf.clear();

	if constexpr (std::is_same_v<std::filesystem::path::string_type, std::string>) {
		// buf.insert(buf.end(), path.native().data(), path.native().data() + path.native().size());
		// char_star(buf, path.native());

		buf.assign(path.native().data(), path.native().data() + path.native().size());
	}
	else if (std::is_same_v<std::filesystem::path::string_type, std::wstring>) {
		// buf = path.string();//char_star(buf, path.native());
#if _WIN32
		buf.clear();
		utf8::unchecked::utf16to8(
			path.native().data(), path.native().data() + path.native().size(), std::back_inserter(buf));
#else
		buf = path.string();
#endif
	}
	else if (std::is_same_v<std::filesystem::path::string_type, std::u8string>) {
		buf.assign(path.native().data(), path.native().data() + path.native().size());
	}
	else if (std::is_same_v<std::filesystem::path::string_type, std::u16string>) {
		buf.clear();
		utf8::unchecked::utf16to8(
			path.native().data(), path.native().data() + path.native().size(), std::back_inserter(buf));
	}
	else if (std::is_same_v<std::filesystem::path::string_type, std::u32string>) {
		buf.clear();
		utf8::unchecked::utf32to8(
			path.native().data(), path.native().data() + path.native().size(), std::back_inserter(buf));
	}
	else {
		buf = path.string();
	}
}

//<x> <y> <w> <h> <label>
int main()
{
	const char *folder_path = "Z:\\Training photos\\SMC21_31_23";
	std::error_code ec{};
	std::filesystem::directory_iterator iterator = {folder_path, ec};

	std::filesystem::path p;
	p.stem();
	p.extension();

	std::string tmp;
	std::string buf;
	if (ec == std::error_code{}) {
		for (const std::filesystem::directory_entry& entry : iterator) {
			//std::cout << entry.path() << '\n';
			//std::string entry_string = entry.path().string();
			//std::cout << entry_string << '\n';
			string_from(tmp, entry.path());

			for (size_t i = 0; i < tmp.size(); i++)
			{
				tmp[i] = std::tolower(tmp[i]);//(tmp[i] < 0x7f) ? (tmp[i] | 32) : tmp[i];
			}

			std::cout << tmp << '\n';
			std::string_view ext = util::utf8::extension(tmp);
			std::cout << ext << '\n';

			if (ext == ".jpg_lbl") { //.PNG .PnG 
				// process this file~
				slurp(entry.path(), buf);

				std::cout << buf.data() << '\n';
			}
		}
	}

	return 0;
}
