#include <catch.hpp>

#include <index.hpp>
#include <source.hpp>
#include <version.hpp>

#include <errors.hpp>

using namespace std;

static const char *M = "[source]";

TEST_CASE("source platform", M) {
  Source src({}, "url");
  REQUIRE(src.platform() == Platform::GenericPlatform);

  src.setPlatform(Platform::UnknownPlatform);
  REQUIRE(src.platform() == Platform::UnknownPlatform);

  src.setPlatform(Platform::WindowsPlatform);
  REQUIRE(src.platform() == Platform::WindowsPlatform);
}

TEST_CASE("source type override", M) {
  Source src({}, "url");
  REQUIRE(src.type() == Package::UnknownType);
  REQUIRE(src.typeOverride() == Package::UnknownType);

  src.setTypeOverride(Package::ScriptType);
  REQUIRE(src.type() == Package::ScriptType);
  REQUIRE(src.typeOverride() == Package::ScriptType);
}

TEST_CASE("source type from package", M) {
  Package pack(Package::EffectType, "package name");
  Version ver("1.0", &pack);
  Source src({}, "url", &ver);

  REQUIRE(src.version() == &ver);

  REQUIRE(src.type() == Package::EffectType);
  src.setTypeOverride(Package::ScriptType);
  REQUIRE(src.type() == Package::ScriptType);
}

TEST_CASE("empty source file name and no package", M) {
  const Source source({}, "url");

  try {
    (void)source.file();
    FAIL();
  }
  catch(const reapack_error &e) {
    REQUIRE(string(e.what()) == "empty source file name and no package");
  }
}

TEST_CASE("parse file section", M) {
  REQUIRE(-1 == Source::getSection("true"));
  REQUIRE(0 == Source::getSection("hello"));
  REQUIRE(Source::MainSection == Source::getSection("main"));
  REQUIRE(Source::MIDIEditorSection == Source::getSection("midi_editor"));
}

TEST_CASE("explicit source section", M) {
  SECTION("script type override") {
    Source source("filename", "url");
    REQUIRE(source.sections() == 0);

    source.setTypeOverride(Package::ScriptType);
    source.setSections(Source::MainSection | Source::MIDIEditorSection);
    REQUIRE(source.sections() == (Source::MainSection | Source::MIDIEditorSection));
  }

  SECTION("other type override") {
    Source source("filename", "url");
    source.setTypeOverride(Package::EffectType);
    source.setSections(Source::MainSection);
    REQUIRE(source.sections() == 0);
  }

  SECTION("package type") {
    Package pack(Package::ScriptType, "package name");
    Version ver("1.0", &pack);
    Source source("filename", "url", &ver);
    source.setSections(Source::MainSection);
    REQUIRE(source.sections() == Source::MainSection);
  }

  SECTION("no package, no type override") {
    Source source("filename", "url");
    source.setSections(Source::MainSection);
    // should not crash!
  }
}

TEST_CASE("implicit source section") {
  SECTION("main") {
    Category cat("Category Name");
    Package pack(Package::ScriptType, "package name", &cat);
    Version ver("1.0", &pack);

    Source source("filename", "url", &ver);
    source.setSections(Source::ImplicitSection);
    REQUIRE(source.sections() == Source::MainSection);
  }

  SECTION("midi editor") {
    Category cat("MIDI Editor");
    Package pack(Package::ScriptType, "package name", &cat);
    Version ver("1.0", &pack);

    Source source("filename", "url", &ver);
    source.setSections(Source::ImplicitSection);
    REQUIRE(source.sections() == Source::MIDIEditorSection);
  }

  SECTION("no category") {
    Source source("filename", "url");
    source.setTypeOverride(Package::ScriptType);

    try {
      source.setSections(Source::ImplicitSection);
      FAIL(); // should throw (or crash if buggy, but not do nothing)
    }
    catch(const reapack_error &) {}
  }
}

TEST_CASE("implicit section detection", M) {
  REQUIRE(Source::MainSection == Source::detectSection("Hello World"));
  REQUIRE(Source::MainSection == Source::detectSection("Hello/World"));
  REQUIRE(Source::MainSection == Source::detectSection("Hello/midi editor"));

  REQUIRE(Source::MIDIEditorSection == Source::detectSection("midi editor"));
  REQUIRE(Source::MIDIEditorSection == Source::detectSection("midi editor/Hello"));
}

TEST_CASE("empty source url", M) {
  try {
    const Source source("filename", {});
    FAIL();
  }
  catch(const reapack_error &e) {
    REQUIRE(string(e.what()) == "empty source url");
  }
}

TEST_CASE("full name without version", M) {
  SECTION("with file name") {
    const Source source("path/to/file", "b");
    REQUIRE(source.fullName() == "file");
  }

  SECTION("without file name") {
    try {
      const Source source({}, "b");
      (void)source.fullName();
      FAIL();
    }
    catch(const reapack_error &e) {
      REQUIRE(string(e.what()) == "empty source file name and no package");
    }
  }
}

TEST_CASE("full name with version", M) {
  SECTION("with file name") {
    Version ver("1.0");
    const Source source("path/to/file", "b", &ver);

    REQUIRE(source.fullName() == "v1.0 (file)");
  }

  SECTION("without file name") {
    Version ver("1.0");
    const Source source({}, "b", &ver);

    REQUIRE(source.fullName() == ver.fullName());
  }
}

TEST_CASE("source target path", M) {
  Index ri("Index Name");
  Category cat("Category Name", &ri);
  Package pack(Package::UnknownType, "package name", &cat);
  Version ver("1.0", &pack);

  Source source("file.name", "url", &ver);

  SECTION("script") {
    source.setTypeOverride(Package::ScriptType);
    const Path expected("Scripts/Index Name/Category Name/file.name");
    REQUIRE(source.targetPath() == expected);
  }

  SECTION("effect") {
    source.setTypeOverride(Package::EffectType);
    const Path expected("Effects/Index Name/Category Name/file.name");
    REQUIRE(source.targetPath() == expected);
  }

  SECTION("extension") {
    source.setTypeOverride(Package::ExtensionType);
    const Path expected("UserPlugins/file.name");
    REQUIRE(source.targetPath() == expected);
  }

  SECTION("data") {
    source.setTypeOverride(Package::DataType);
    const Path expected("Data/file.name");
    REQUIRE(source.targetPath() == expected);
  }

  SECTION("theme") {
    source.setTypeOverride(Package::ThemeType);
    const Path expected("ColorThemes/file.name");
    REQUIRE(source.targetPath() == expected);
  }
}

TEST_CASE("target path with parent directory traversal", M) {
  Index ri("Index Name");
  Category cat("Category Name", &ri);
  Package pack(Package::ScriptType, "package name", &cat);
  Version ver("1.0", &pack);
  Source source("../../../file.name", "url", &ver);

  SECTION("script") {
    Path expected;
    expected.append("Scripts");
    expected.append("Index Name");
    // expected.append("Category Name"); // only the category can be bypassed!
    expected.append("file.name");

    REQUIRE(source.targetPath() == expected);
  }

  SECTION("extension") {
    Path expected;
    expected.append("UserPlugins");
    expected.append("file.name");
    source.setTypeOverride(Package::ExtensionType);
    REQUIRE(source.targetPath() == expected);
  }
}

TEST_CASE("target path without package", M) {
  try {
    const Source source("a", "b");
    (void)source.targetPath();
    FAIL();
  }
  catch(const reapack_error &e) {
    REQUIRE(string(e.what()) == "no package associated with this source");
  }
}

TEST_CASE("target path for unknown package type", M) {
  Index ri("name");
  Category cat("name", &ri);
  Package pack(Package::UnknownType, "a", &cat);
  Version ver("1.0", &pack);
  Source src({}, "url", &ver);

  REQUIRE(src.targetPath().empty());
}

TEST_CASE("directory traversal in category name", M) {
  Index ri("Remote Name");
  Category cat("../..", &ri);
  Package pack(Package::ScriptType, "file.name", &cat);
  Version ver("1.0", &pack);
  Source src({}, "url", &ver);

  Path expected;
  expected.append("Scripts");
  expected.append("Remote Name");
  expected.append("file.name");

  REQUIRE(src.targetPath() == expected);
}

TEST_CASE("target path without category", M) {
  Package pack(Package::ScriptType, "file.name");
  Version ver("1.0", &pack);
  Source src({}, "url", &ver);

  try {
    src.targetPath();
    FAIL();
  }
  catch(const reapack_error &e) {
    REQUIRE(string(e.what()) == "category or index is unset");
  }
}
