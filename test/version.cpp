#include <catch.hpp>

#include "helper/io.hpp"

#include <version.hpp>

#include <errors.hpp>
#include <index.hpp>
#include <package.hpp>

using namespace std;

#define MAKE_VERSION \
  Index ri("Remote Name"); \
  Category cat("Category Name", &ri); \
  Package pkg(Package::ScriptType, "Hello", &cat); \
  Version ver("1", &pkg);

static Source *mksource(Source::Platform p, Version *parent)
{
  Source *src = new Source("file", "url", parent);
  src->setPlatform(p);
  return src;
}

static const char *M = "[version]";

TEST_CASE("construct null version", M) {
  const Version ver;

  REQUIRE(ver.size() == 0);
  REQUIRE_FALSE(ver.isPrerelease());
  REQUIRE(ver.displayTime().empty());
  REQUIRE(ver.package() == nullptr);
  REQUIRE(ver.mainSource() == nullptr);
}

TEST_CASE("parse version", M) {
  Version ver;

  SECTION("valid") {
    ver.parse("1.0.1");
    REQUIRE(ver.name() == "1.0.1");
    REQUIRE(ver.size() == 3);
  }

  SECTION("prerelease set/unset") {
    REQUIRE_FALSE(ver.isPrerelease());
    ver.parse("1.0beta");
    REQUIRE(ver.isPrerelease());
    ver.parse("1.0");
    REQUIRE_FALSE(ver.isPrerelease());
  }

  SECTION("invalid") {
    try { ver.parse("hello"); FAIL(); }
    catch(const reapack_error &) {}

    REQUIRE(ver.name().empty());
    REQUIRE(ver.size() == 0);
  }
}

TEST_CASE("parse version failsafe", M) {
  Version ver;

  SECTION("valid") {
    REQUIRE(ver.tryParse("1.0"));

    REQUIRE(ver.name() == "1.0");
    REQUIRE(ver.size() == 2);
  }

  SECTION("invalid") {
    REQUIRE_FALSE(ver.tryParse("hello"));

    REQUIRE(ver.name().empty());
    REQUIRE(ver.size() == 0);
  }
}

TEST_CASE("construct invalid version", M) {
  try {
    Version ver("hello");
    FAIL();
  }
  catch(const reapack_error &e) {
    REQUIRE(string(e.what()) == "invalid version name");
  }
}

TEST_CASE("decimal version", M) {
  Version ver("5.05");
  REQUIRE(ver == Version("5.5"));
  REQUIRE(ver < Version("5.50"));
}

TEST_CASE("5 version segments", M) {
  REQUIRE(Version("1.1.1.1.0") < Version("1.1.1.1.1"));
  REQUIRE(Version("1.1.1.1.1") == Version("1.1.1.1.1"));
  REQUIRE(Version("1.1.1.1.1") < Version("1.1.1.1.2"));
  REQUIRE(Version("1.1.1.1.1") < Version("1.1.1.2.0"));
}

TEST_CASE("version segment overflow", M) {
  try {
    Version ver("9999999999999999999999");
    FAIL();
  }
  catch(const reapack_error &e) {
    REQUIRE(string(e.what()) == "version segment overflow");
  }
}

TEST_CASE("compare versions", M) {
  SECTION("equality") {
    REQUIRE(Version("1.0").compare(Version("1.0")) == 0);

    REQUIRE(Version("1.0") == Version("1.0"));
    REQUIRE_FALSE(Version("1.0") == Version("1.1"));
  }

  SECTION("inequality") {
    REQUIRE_FALSE(Version("1.0") != Version("1.0"));
    REQUIRE(Version("1.0") != Version("1.1"));
  }

  SECTION("less than") {
    REQUIRE(Version("1.0").compare(Version("1.1")) == -1);

    REQUIRE(Version("1.0") < Version("1.1"));
    REQUIRE_FALSE(Version("1.0") < Version("1.0"));
    REQUIRE_FALSE(Version("1.1") < Version("1.0"));
  }

  SECTION("less than or equal") {
    REQUIRE(Version("1.0") <= Version("1.1"));
    REQUIRE(Version("1.0") <= Version("1.0"));
    REQUIRE_FALSE(Version("1.1") <= Version("1.0"));
  }

  SECTION("greater than") {
    REQUIRE(Version("1.1").compare(Version("1.0")) == 1);

    REQUIRE_FALSE(Version("1.0") > Version("1.1"));
    REQUIRE_FALSE(Version("1.0") > Version("1.0"));
    REQUIRE(Version("1.1") > Version("1.0"));
  }

  SECTION("greater than or equal") {
    REQUIRE_FALSE(Version("1.0") >= Version("1.1"));
    REQUIRE(Version("1.0") >= Version("1.0"));
    REQUIRE(Version("1.1") >= Version("1.0"));
  }
}

TEST_CASE("compare versions with more or less segments", M) {
  REQUIRE(Version("1") == Version("1.0.0.0"));
  REQUIRE(Version("1") != Version("1.0.0.1"));

  REQUIRE(Version("1.0.0.0") == Version("1"));
  REQUIRE(Version("1.0.0.1") != Version("1"));
}

TEST_CASE("prerelease versions", M) {
  SECTION("detect") {
    REQUIRE_FALSE(Version("1.0").isPrerelease());
    REQUIRE(Version("1.0b").isPrerelease());
    REQUIRE(Version("1.0-beta").isPrerelease());
    REQUIRE(Version("1.0-beta1").isPrerelease());
  }

  SECTION("compare") {
    REQUIRE(Version("0.9") < Version("1.0a"));
    REQUIRE(Version("1.0a.2") < Version("1.0b.1"));
    REQUIRE(Version("1.0-beta1") < Version("1.0"));

    REQUIRE(Version("1.0b") < Version("1.0.1"));
    REQUIRE(Version("1.0.1") > Version("1.0b"));
  }
}

TEST_CASE("version full name", M) {
  SECTION("no package") {
    Version ver("1.0");
    REQUIRE(ver.fullName() == "v1.0");
  }

  SECTION("with package") {
    Package pkg(Package::UnknownType, "file.name");
    Version ver("1.0", &pkg);

    REQUIRE(ver.fullName() == "file.name v1.0");
  }

  SECTION("with category") {
    Category cat("Category Name");
    Package pkg(Package::UnknownType, "file.name", &cat);
    Version ver("1.0", &pkg);

    REQUIRE(ver.fullName() == "Category Name/file.name v1.0");
  }

  SECTION("with index") {
    Index ri("Remote Name");
    Category cat("Category Name", &ri);
    Package pkg(Package::UnknownType, "file.name", &cat);
    Version ver("1.0", &pkg);

    REQUIRE(ver.fullName() == "Remote Name/Category Name/file.name v1.0");
  }
}

TEST_CASE("add source", M) {
  MAKE_VERSION

  CHECK(ver.sources().size() == 0);

  Source *src = new Source("a", "b", &ver);
  ver.addSource(src);

  CHECK(ver.mainSource() == nullptr);
  CHECK(ver.sources().size() == 1);

  REQUIRE(src->version() == &ver);
  REQUIRE(ver.source(0) == src);
}

TEST_CASE("add owned source", M) {
  MAKE_VERSION

  Version ver2("1");
  Source *src = new Source("a", "b", &ver2);

  try {
    ver.addSource(src);
    FAIL();
  }
  catch(const reapack_error &e) {
    delete src;
    REQUIRE(string(e.what()) == "source belongs to another version");
  }
}

TEST_CASE("add main source", M) {
  MAKE_VERSION

  Source *src = new Source({}, "b", &ver);
  ver.addSource(src);

  REQUIRE(ver.mainSource() == src);
}

TEST_CASE("list files", M) {
  MAKE_VERSION

  Source *src1 = new Source("file", "url", &ver);
  ver.addSource(src1);

  Path path1;
  path1.append("Scripts");
  path1.append("Remote Name");
  path1.append("Category Name");
  path1.append("file");

  const set<Path> expected{path1};
  REQUIRE(ver.files() == expected);
}

TEST_CASE("drop sources for unknown platforms", M) {
  MAKE_VERSION
  Source *src = new Source("a", "b", &ver);
  src->setPlatform(Source::UnknownPlatform);
  ver.addSource(src);

  REQUIRE(ver.sources().size() == 0);
}

TEST_CASE("version author", M) {
  Version ver("1.0");
  CHECK(ver.author().empty());

  ver.setAuthor("cfillion");
  REQUIRE(ver.author() == "cfillion");
}

TEST_CASE("version date", M) {
  Version ver("1.0");
  CHECK(ver.time().tm_year == 0);
  CHECK(ver.time().tm_mon == 0);
  CHECK(ver.time().tm_mday == 0);
  CHECK(ver.displayTime() == "");

  SECTION("valid") {
    ver.setTime("2016-02-12T01:16:40Z");
    REQUIRE(ver.time().tm_year == 2016 - 1900);
    REQUIRE(ver.time().tm_mon == 2 - 1);
    REQUIRE(ver.time().tm_mday == 12);
    REQUIRE(ver.displayTime() != "");
  }

  SECTION("garbage") {
    ver.setTime("hello world");
    REQUIRE(ver.time().tm_year == 0);
    REQUIRE(ver.displayTime() == "");
  }

  SECTION("out of range") {
    ver.setTime("2016-99-99T99:99:99Z");
    ver.displayTime(); // no crash
  }
}

TEST_CASE("copy version constructor", M) {
  const Package pkg(Package::UnknownType, "Hello");

  Version original("1.1b", &pkg);
  original.setAuthor("John Doe");
  original.setChangelog("Initial release");
  original.setTime("2016-02-12T01:16:40Z");

  const Version copy1(original);
  REQUIRE(copy1.name() == "1.1b");
  REQUIRE(copy1.size() == original.size());
  REQUIRE(copy1.isPrerelease() == original.isPrerelease());
  REQUIRE(copy1.author() == original.author());
  REQUIRE(copy1.changelog() == original.changelog());
  REQUIRE(copy1.displayTime() == original.displayTime());
  REQUIRE(copy1.package() == nullptr);
  REQUIRE(copy1.mainSource() == nullptr);
  REQUIRE(copy1.sources().empty());

  const Version copy2(original, &pkg);
  REQUIRE(copy2.package() == &pkg);
}

#ifdef __APPLE__
TEST_CASE("drop windows sources on os x", M) {
  MAKE_VERSION

  ver.addSource(mksource(Source::WindowsPlatform, &ver));
  ver.addSource(mksource(Source::Win32Platform, &ver));
  ver.addSource(mksource(Source::Win64Platform, &ver));

  REQUIRE(ver.sources().size() == 0);
}

#ifdef __x86_64__
TEST_CASE("drop 32-bit sources on os x 64-bit", M) {
  MAKE_VERSION
  ver.addSource(mksource(Source::Darwin32Platform, &ver));

  REQUIRE(ver.sources().size() == 0);
}

TEST_CASE("valid sources for os x 64-bit", M) {
  MAKE_VERSION

  ver.addSource(mksource(Source::GenericPlatform, &ver));
  ver.addSource(mksource(Source::DarwinPlatform, &ver));
  ver.addSource(mksource(Source::Darwin64Platform, &ver));

  REQUIRE(ver.sources().size() == 3);
}
#else
TEST_CASE("drop 64-bit sources on os x 32-bit", M) {
  MAKE_VERSION
  ver.addSource(mksource(Source::Darwin64Platform, &ver));

  REQUIRE(ver.sources().size() == 0);
}

TEST_CASE("valid sources for os x 32-bit", M) {
  MAKE_VERSION

  ver.addSource(mksource(Source::GenericPlatform, &ver));
  ver.addSource(mksource(Source::DarwinPlatform, &ver));
  ver.addSource(mksource(Source::Darwin32Platform, &ver));

  REQUIRE(ver.sources().size() == 3);
}
#endif

#elif _WIN32
TEST_CASE("drop os x sources on windows", M) {
  MAKE_VERSION

  ver.addSource(mksource(Source::DarwinPlatform, &ver));
  ver.addSource(mksource(Source::Darwin32Platform, &ver));
  ver.addSource(mksource(Source::Darwin64Platform, &ver));

  REQUIRE(ver.sources().size() == 0);
}

#ifdef _WIN64
TEST_CASE("drop 32-bit sources on windows 64-bit", M) {
  MAKE_VERSION
  ver.addSource(mksource(Source::Win32Platform, &ver));

  REQUIRE(ver.sources().size() == 0);
}

TEST_CASE("valid sources for windows 64-bit", M) {
  MAKE_VERSION

  ver.addSource(mksource(Source::GenericPlatform, &ver));
  ver.addSource(mksource(Source::WindowsPlatform, &ver));
  ver.addSource(mksource(Source::Win64Platform, &ver));

  REQUIRE(ver.sources().size() == 3);
}
#else
TEST_CASE("drop 64-bit sources on windows 32-bit", M) {
  MAKE_VERSION
  ver.addSource(mksource(Source::Win64Platform, &ver));

  REQUIRE(ver.sources().size() == 0);
}

TEST_CASE("valid sources for windows 32-bit", M) {
  MAKE_VERSION

  ver.addSource(mksource(Source::GenericPlatform, &ver));
  ver.addSource(mksource(Source::WindowsPlatform, &ver));
  ver.addSource(mksource(Source::Win32Platform, &ver));

  REQUIRE(ver.sources().size() == 3);
}
#endif
#endif
