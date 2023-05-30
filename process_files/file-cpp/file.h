#pragma once

#include <string>
#include <string_view>
#include <algorithm>
#include <cctype>

namespace util {
	namespace wide {
		_NODISCARD inline bool is_drive_prefix(const wchar_t* const _First) {
			// test if _First points to a prefix of the form X:
			// pre: _First points to at least 2 wchar_t instances
			// pre: Little endian
			/*
			auto _Value = _Unaligned_load<unsigned int>(_First);
			_Value &= 0xFFFF'FFDFu; // transform lowercase drive letters into uppercase ones
			_Value -= (static_cast<unsigned int>(L':') << (sizeof(wchar_t) * CHAR_BIT)) | L'A';
			return _Value < 26;
			*/
			return *_First == L':' && ((std::tolower(_First[1]) - L'a') < 26);
		}

		_NODISCARD inline bool has_drive_letter_prefix(const wchar_t* const _First, const wchar_t* const _Last) {
			// test if [_First, _Last) has a prefix of the form X:
			return _Last - _First >= 2 && is_drive_prefix(_First);
		}

		constexpr bool is_slash(wchar_t c) {
			return c == L'\\' || c == L'/';
		}

		const wchar_t* find_root_name_end(const wchar_t* const _First, const wchar_t* const _Last) {
			// attempt to parse [_First, _Last) as a path and return the end of root-name if it exists; otherwise, _First

			// This is the place in the generic grammar where library implementations have the most freedom.
			// Below are example Windows paths, and what we've decided to do with them:
			// * X:DriveRelative, X:\DosAbsolute
			//   We parse X: as root-name, if and only if \ is present we consider that root-directory
			// * \RootRelative
			//   We parse no root-name, and \ as root-directory
			// * \\server\share
			//   We parse \\server as root-name, \ as root-directory, and share as the first element in relative-path.
			//   Technically, Windows considers all of \\server\share the logical "root", but for purposes
			//   of decomposition we want those split, so that path(R"(\\server\share)").replace_filename("other_share")
			//   is \\server\other_share
			// * \\?\device
			// * \??\device
			// * \\.\device
			//   CreateFile appears to treat these as the same thing; we will set the first three characters as root-name
			//   and the first \ as root-directory. Support for these prefixes varies by particular Windows version, but
			//   for the purposes of path decomposition we don't need to worry about that.
			// * \\?\UNC\server\share
			//   MSDN explicitly documents the \\?\UNC syntax as a special case. What actually happens is that the device
			//   Mup, or "Multiple UNC provider", owns the path \\?\UNC in the NT namespace, and is responsible for the
			//   network file access. When the user says \\server\share, CreateFile translates that into
			//   \\?\UNC\server\share to get the remote server access behavior. Because NT treats this like any other
			//   device, we have chosen to treat this as the \\?\ case above.
			if (_Last - _First < 2) {
				return _First;
			}

			if (has_drive_letter_prefix(_First, _Last)) { // check for X: first because it's the most common root-name
				return _First + 2;
			}

			if (!is_slash(_First[0])) { // all the other root-names start with a slash; check that first because
				// we expect paths without a leading slash to be very common
				return _First;
			}

			// $ means anything other than a slash, including potentially the end of the input
			if (_Last - _First >= 4 && is_slash(_First[3]) && (_Last - _First == 4 || !is_slash(_First[4])) // \xx\$
				&& ((is_slash(_First[1]) && (_First[2] == L'?' || _First[2] == L'.')) // \\?\$ or \\.\$
					|| (_First[1] == L'?' && _First[2] == L'?'))) { // \??\$
				return _First + 3;
			}

			if (_Last - _First >= 3 && is_slash(_First[1]) && !is_slash(_First[2])) { // \\server
				return std::find_if(_First + 3, _Last, is_slash);
			}

			// no match
			return _First;
		}

		std::wstring_view root_name(const std::wstring_view path) {
			// attempt to parse path as a path and return the root-name if it exists; otherwise, an empty view
			const auto data = path.data();
			const auto tail = data + path.size();
			return std::wstring_view(data, static_cast<size_t>(find_root_name_end(data, tail) - data));
		}
	}
	namespace utf8 {
		bool is_drive_prefix(const char* const _First) {
			// test if _First points to a prefix of the form X:
			// pre: _First points to at least 2 char instances
			// pre: Little endian
			/*
			auto _Value = _Unaligned_load<unsigned short>(_First);
			_Value &= 0xFFFF'FFDFu; // transform lowercase drive letters into uppercase ones
			_Value -= (static_cast<unsigned short>(L':') << (sizeof(char) * CHAR_BIT)) | L'A';
			return _Value < 26;
			*/
			return *_First == ':' && ((std::tolower(_First[1]) - 'a') < 26);
		}

		bool has_drive_letter_prefix(const char* const _First, const char* const _Last) {
			// test if [_First, _Last) has a prefix of the form X:
			return _Last - _First >= 2 && is_drive_prefix(_First);
		}

		constexpr bool is_slash(char c) {
			return c == '\\' || c == '/';
		}

		const char* find_root_name_end(const char* const _First, const char* const _Last) {
			// attempt to parse [_First, _Last) as a path and return the end of root-name if it exists; otherwise, _First

			// This is the place in the generic grammar where library implementations have the most freedom.
			// Below are example Windows paths, and what we've decided to do with them:
			// * X:DriveRelative, X:\DosAbsolute
			//   We parse X: as root-name, if and only if \ is present we consider that root-directory
			// * \RootRelative
			//   We parse no root-name, and \ as root-directory
			// * \\server\share
			//   We parse \\server as root-name, \ as root-directory, and share as the first element in relative-path.
			//   Technically, Windows considers all of \\server\share the logical "root", but for purposes
			//   of decomposition we want those split, so that path(R"(\\server\share)").replace_filename("other_share")
			//   is \\server\other_share
			// * \\?\device
			// * \??\device
			// * \\.\device
			//   CreateFile appears to treat these as the same thing; we will set the first three characters as root-name
			//   and the first \ as root-directory. Support for these prefixes varies by particular Windows version, but
			//   for the purposes of path decomposition we don't need to worry about that.
			// * \\?\UNC\server\share
			//   MSDN explicitly documents the \\?\UNC syntax as a special case. What actually happens is that the device
			//   Mup, or "Multiple UNC provider", owns the path \\?\UNC in the NT namespace, and is responsible for the
			//   network file access. When the user says \\server\share, CreateFile translates that into
			//   \\?\UNC\server\share to get the remote server access behavior. Because NT treats this like any other
			//   device, we have chosen to treat this as the \\?\ case above.
			if (_Last - _First < 2) {
				return _First;
			}

			if (has_drive_letter_prefix(_First, _Last)) { // check for X: first because it's the most common root-name
				return _First + 2;
			}

			if (!is_slash(_First[0])) { // all the other root-names start with a slash; check that first because
				// we expect paths without a leading slash to be very common
				return _First;
			}

			// $ means anything other than a slash, including potentially the end of the input
			if (_Last - _First >= 4 && is_slash(_First[3]) && (_Last - _First == 4 || !is_slash(_First[4])) // \xx\$
				&& ((is_slash(_First[1]) && (_First[2] == '?' || _First[2] == '.')) // \\?\$ or \\.\$
					|| (_First[1] == '?' && _First[2] == '?'))) { // \??\$
				return _First + 3;
			}

			if (_Last - _First >= 3 && is_slash(_First[1]) && !is_slash(_First[2])) { // \\server
				return std::find_if(_First + 3, _Last, is_slash);
			}

			// no match
			return _First;
		}


		std::string_view root_name(const std::string_view path) {
			// attempt to parse path as a path and return the root-name if it exists; otherwise, an empty view
			const auto data = path.data();
			const auto tail = data + path.size();
			return std::string_view(data, static_cast<size_t>(find_root_name_end(data, tail) - data));
		}

		const char* find_relative_path(const char* const _First, const char* const _Last) {
			// attempt to parse [_First, _Last) as a path and return the start of relative-path
			return std::find_if_not(find_root_name_end(_First, _Last), _Last, is_slash);
		}

		std::string_view relative_path(const std::string_view path) {
			// attempt to parse path as a path and return the relative-path if it exists; otherwise, an empty view
			const auto data = path.data();
			const auto tail = data + path.size();
			const auto relative_path = find_relative_path(data, tail);
			return std::string_view(relative_path, static_cast<size_t>(tail - relative_path));
		}

		std::string_view parent_path(const std::string_view path) {
			// attempt to parse path as a path and return the parent_path if it exists; otherwise, an empty view
			const auto data = path.data();
			auto tail = data + path.size();
			const auto rel_path = find_relative_path(data, tail);
			// case 1: relative-path ends in a directory-separator, remove the separator to remove "magic empty path"
			//  for example: R"(/cat/dog/\//\)"
			// case 2: relative-path doesn't end in a directory-separator, remove the filename and last directory-separator
			//  to prevent creation of a "magic empty path"
			//  for example: "/cat/dog"
			while (rel_path != tail && !is_slash(tail[-1])) {
				// handle case 2 by removing trailing filename, puts us into case 1
				--tail;
			}

			while (rel_path != tail && is_slash(tail[-1])) { // handle case 1 by removing trailing slashes
				--tail;
			}

			return std::string_view(data, static_cast<size_t>(tail - data));
		}

		inline const char* find_filename(const char* const path, const char* path_end) {
			// attempt to parse [path, path_end) as a path and return the start of filename if it exists; otherwise, path_end
			const auto relative_path = find_relative_path(path, path_end);
			while (relative_path != path_end && !is_slash(path_end[-1])) {
				--path_end;
			}

			return path_end;
		}

		inline std::string_view filename(const std::string_view path) {
			// attempt to parse path as a path and return the filename if it exists; otherwise, an empty view
			const auto data = path.data();
			const auto tail = data + path.size();
			const auto f = find_filename(data, tail);
			return std::string_view(f, static_cast<size_t>(tail - f));
		}

		constexpr const char* find_extension(const char* const filename, const char* const additional) {
			// find dividing point between stem and extension in a generic format filename consisting of [filename, additional)
			auto extension = additional;
			if (filename == extension) { // empty path
				return additional;
			}

			--extension;
			if (filename == extension) {
				// path is length 1 and either dot, or has no dots; either way, extension() is empty
				return additional;
			}

			if (*extension == '.') { // we might have found the end of stem
				if (filename == extension - 1 && extension[-1] == '.') { // dotdot special case
					return additional;
				}
				else { // x.
					return extension;
				}
			}

			while (filename != --extension) {
				if (*extension == '.') { // found a dot which is not in first position, so it starts extension()
					return extension;
				}
			}

			// if we got here, either there are no dots, in which case extension is empty, or the first element
			// is a dot, in which case we have the leading single dot special case, which also makes extension empty
			return additional;
		}

		std::string_view stem(const std::string_view path) {
			// attempt to parse path as a path and return the stem if it exists; otherwise, an empty view
			const auto data = path.data();
			const auto tail = data + path.size();
			const auto fname = find_filename(data, tail);
			const auto ads = std::find(fname, tail, ':'); // strip alternate data streams in intra-filename decomposition
			const auto exts = find_extension(fname, ads);
			return std::string_view(fname, static_cast<size_t>(exts - fname));
		}

		std::string_view extension(const std::string_view path) {
			// attempt to parse path as a path and return the extension if it exists; otherwise, an empty view
			const auto data = path.data();
			const auto tail = data + path.size();
			const auto fname = find_filename(data, tail);
			const auto addtional =
				std::find(fname, tail, ':'); // strip alternate data streams in intra-filename decomposition
			const auto exts = find_extension(fname, addtional);
			return std::string_view(exts, static_cast<size_t>(addtional - exts));
		}

	}
}