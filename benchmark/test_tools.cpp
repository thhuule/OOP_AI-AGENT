#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include "tools/ToolRegistry.h"
#include "tools/CalculatorTool.h"
#include "tools/FileTool.h"
#include "tools/ExecTool.h"
#include "tools/WebSearchTool.h"
#include "tools/MemoryTool.h"
#include "tools/TimeTool.h"
#include "tools/JsonTool.h"
#include "tools/GitTool.h"
#include "tools/Embedding.h"
#include "tools/ScreenshotTool.h"
#include "tools/ActionTool.h"

using namespace oop_agent;

void test_registry_instance_registration() {
    std::cout << "[TEST] Running test_registry_instance_registration...\n";
    ToolRegistry reg;
    
    // Test null registration
    assert(!reg.register_tool(nullptr));
    
    // Test valid registration
    auto calc = std::make_shared<CalculatorTool>();
    assert(reg.register_tool(calc));
    assert(reg.has_instance("calculator"));
    
    // Lookup returns pointer to registered instance
    assert(reg.lookup("calculator") != nullptr);
    assert(reg.lookup("calculator")->get_name() == "calculator");
    
    // Duplicate instance registration returns false
    auto calc2 = std::make_shared<CalculatorTool>();
    assert(!reg.register_tool(calc2));
    
    std::cout << "  -> PASSED\n";
}

void test_factory_creation() {
    std::cout << "[TEST] Running test_factory_creation...\n";
    ToolRegistry reg;
    
    assert(reg.register_creator("calculator", []() { return std::make_unique<CalculatorTool>(); }));
    assert(reg.has_creator("calculator"));
    
    // Create produces fresh distinct instances
    auto t1 = reg.create("calculator");
    auto t2 = reg.create("calculator");
    assert(t1 != nullptr);
    assert(t2 != nullptr);
    assert(t1.get() != t2.get());
    
    // Unknown creator returns nullptr
    auto unknown = reg.create("unknown_tool");
    assert(unknown == nullptr);
    
    std::cout << "  -> PASSED\n";
}

void test_aliases_and_normalization() {
    std::cout << "[TEST] Running test_aliases_and_normalization...\n";
    ToolRegistry reg;
    
    reg.register_creator("execute_shell", []() { return std::make_unique<ExecTool>(); });
    reg.register_alias("exec", "execute_shell");
    
    assert(reg.normalize("exec") == "execute_shell");
    assert(reg.normalize("unknown") == "unknown");
    assert(reg.has_creator("exec"));
    
    auto t = reg.create("exec");
    assert(t != nullptr);
    assert(t->get_name() == "execute_shell");
    
    std::cout << "  -> PASSED\n";
}

void test_allow_deny_policies() {
    std::cout << "[TEST] Running test_allow_deny_policies...\n";
    ToolRegistry reg;
    reg.register_all_tools();
    
    // Deny policy
    reg.deny("execute_shell");
    assert(!reg.is_allowed("execute_shell"));
    assert(reg.lookup("exec") == nullptr);
    assert(reg.create("exec") == nullptr);
    
    // Allow policy (whitelist)
    ToolRegistry reg_allow;
    reg_allow.register_all_tools();
    reg_allow.allow("calculator");
    assert(reg_allow.is_allowed("calculator"));
    assert(!reg_allow.is_allowed("web_search"));
    assert(reg_allow.lookup("calculator") != nullptr);
    assert(reg_allow.lookup("web_search") == nullptr);
    
    std::cout << "  -> PASSED\n";
}

/// B-9.5-02: Focused test — duplicate creator overwrite semantics.
///
/// Theo contract trong ToolRegistry.h:
///   register_creator() ALWAYS overwrites the existing creator for the same name.
/// Điều này khác với register_tool() (instance registry) vốn returns false khi trùng.
///
/// Test matrix:
///   1. Register creator A → create() trả instance của A.
///   2. Register creator B (cùng tên) → creator B overwrite creator A.
///   3. create() sau overwrite trả instance của B, không phải A.
///   4. Hai lần create() liên tiếp trả hai object khác nhau (fresh instance).
///   5. register_creator() returns true cả hai lần (không reject duplicate).
void test_duplicate_creator_overwrite() {
    std::cout << "[TEST] Running test_duplicate_creator_overwrite...\n";
    ToolRegistry reg;

    // --- Round 1: register CalculatorTool creator for "dual_test" ---
    assert(reg.register_creator("dual_test",
        []() { return std::make_unique<CalculatorTool>(); }) == true);
    assert(reg.has_creator("dual_test"));

    auto first = reg.create("dual_test");
    assert(first != nullptr);
    // CalculatorTool reports name "calculator"
    assert(first->get_name() == "calculator");

    // --- Round 2: overwrite with TimeTool creator (same canonical name) ---
    assert(reg.register_creator("dual_test",
        []() { return std::make_unique<TimeTool>(); }) == true);   // overwrite always returns true
    assert(reg.has_creator("dual_test"));

    auto after_overwrite = reg.create("dual_test");
    assert(after_overwrite != nullptr);
    // After overwrite, creator produces TimeTool, not CalculatorTool
    assert(after_overwrite->get_name() == "time");

    // --- Round 3: two consecutive creates → distinct fresh objects ---
    auto obj_a = reg.create("dual_test");
    auto obj_b = reg.create("dual_test");
    assert(obj_a != nullptr);
    assert(obj_b != nullptr);
    assert(obj_a.get() != obj_b.get());  // must be different heap objects

    std::cout << "  -> PASSED\n";
}

void test_register_all_tools() {
    std::cout << "[TEST] Running test_register_all_tools...\n";
    ToolRegistry reg;
    reg.register_all_tools();
    
    // Verify instances
    assert(reg.has_instance("calculator"));
    assert(reg.has_instance("file"));
    assert(reg.has_instance("read_file"));
    assert(reg.has_instance("write_file"));
    assert(reg.has_instance("append_file"));
    assert(reg.has_instance("execute_shell"));
    assert(reg.has_instance("web_search"));
    assert(reg.has_instance("memory"));
    assert(reg.has_instance("time"));
    assert(reg.has_instance("json"));
    assert(reg.has_instance("git"));
    assert(reg.has_instance("capture_screenshot"));
    assert(reg.has_instance("gui_action"));
    
    // Verify aliases
    assert(reg.lookup("calculate") != nullptr);
    assert(reg.lookup("exec") != nullptr);
    assert(reg.lookup("google_search") != nullptr);
    assert(reg.lookup("create_file") != nullptr);
    assert(reg.lookup("file_read") != nullptr);
    assert(reg.lookup("file_write") != nullptr);
    assert(reg.lookup("screenshot") != nullptr);
    
    // Verify tool execution returns valid result
    Tool* calc = reg.lookup("calculator");
    auto res = calc->execute("2 + 3");
    assert(res.has_value());
    assert(res.value().find("5") != std::string::npos);
    
    std::cout << "  -> PASSED\n";
}

void test_tool_error_paths() {
    std::cout << "[TEST] Running test_tool_error_paths...\n";

    // ExecTool: empty argument -> InvalidArgument
    ExecTool exec_tool;
    auto exec_res = exec_tool.execute("");
    assert(!exec_res.has_value());
    assert(exec_res.error() == ToolError::InvalidArgument);

    // GitTool: empty / unallowed subcommand -> InvalidArgument
    GitTool git_tool;
    auto git_empty = git_tool.execute("");
    assert(!git_empty.has_value());
    assert(git_empty.error() == ToolError::InvalidArgument);

    auto git_unallowed = git_tool.execute("push --force");
    assert(!git_unallowed.has_value());
    assert(git_unallowed.error() == ToolError::InvalidArgument);

    // JsonTool: empty -> InvalidArgument, malformed -> ExecutionFailed
    JsonTool json_tool;
    auto json_empty = json_tool.execute("");
    assert(!json_empty.has_value());
    assert(json_empty.error() == ToolError::InvalidArgument);

    auto json_bad = json_tool.execute("{invalid json");
    assert(!json_bad.has_value());
    assert(json_bad.error() == ToolError::ExecutionFailed);

    // MemoryTool: empty / unknown command -> InvalidArgument
    MemoryTool mem_tool;
    auto mem_empty = mem_tool.execute("");
    assert(!mem_empty.has_value());
    assert(mem_empty.error() == ToolError::InvalidArgument);

    auto mem_bad = mem_tool.execute("unknown_command");
    assert(!mem_bad.has_value());
    assert(mem_bad.error() == ToolError::InvalidArgument);

    std::cout << "  -> PASSED\n";
}

/// B-10-01: Canonical names & description contract.
///
/// Mọi tool được expose phải:
///   1. Có get_name() trùng canonical name được lookup trong registry.
///   2. Có description không rỗng và không gọi tên tool không tồn tại
///      (ví dụ legacy "python_interpreter" từ danh sách lỗi lịch sử).
///   3. Alias phải resolve về canonical name thật.
void test_canonical_names_and_descriptions() {
    std::cout << "[TEST] Running test_canonical_names_and_descriptions...\n";
    ToolRegistry reg;
    reg.register_all_tools();

    for (const char* name : {"calculator", "file", "read_file", "write_file",
                             "append_file", "execute_shell", "web_search",
                             "memory", "time", "json", "git",
                             "capture_screenshot", "gui_action"}) {
        Tool* t = reg.lookup(name);
        assert(t != nullptr);
        assert(std::string(t->get_name()) == name);
        assert(!t->get_description().empty());

        const std::string desc(t->get_description());
        assert(desc.find("python_interpreter") == std::string::npos);
    }

    // Aliases must resolve to a registered canonical tool.
    assert(reg.lookup("calculate")->get_name() == "calculator");
    assert(reg.lookup("exec")->get_name() == "execute_shell");
    assert(reg.lookup("google_search")->get_name() == "web_search");
    assert(reg.lookup("create_file")->get_name() == "write_file");
    assert(reg.lookup("file_read")->get_name() == "read_file");
    assert(reg.lookup("file_write")->get_name() == "write_file");
    assert(reg.lookup("screenshot")->get_name() == "capture_screenshot");

    std::cout << "  -> PASSED\n";
}

/// B-10-02: FileWriteTool / FileReadTool args parsing đa format.
///
///   - CSV string:      "result.txt,1081"
///   - JSON {"path"...}   {"path":"notes.txt","content":"Agent test run"}
///   - JSON {"filename"}  {"filename":"capital.txt","content":"Tokyo"}
///   - invalid input phải trả ToolError (std::unexpected), không throw.
void test_file_args_formats() {
    std::cout << "[TEST] Running test_file_args_formats...\n";
    FileWriteTool writer;
    FileAppendTool appender;
    FileReadTool reader;

    // CSV string format: filename,content
    auto csv = writer.execute("result.txt,1081");
    assert(csv.has_value());

    // JSON with "path" key
    auto jpath = writer.execute(R"({"path":"notes.txt","content":"Agent test run"})");
    assert(jpath.has_value());

    // JSON with "filename" key
    auto jfname = writer.execute(R"({"filename":"capital.txt","content":"Tokyo"})");
    assert(jfname.has_value());

    // Append CSV format
    auto append = appender.execute("data.txt,appended");
    assert(append.has_value());

    // Read back with plain path and JSON {"path":...}
    auto plain = reader.execute("result.txt");
    assert(plain.has_value());
    assert(plain.value().find("1081") != std::string::npos);

    auto jread = reader.execute(R"({"path":"capital.txt"})");
    assert(jread.has_value());
    assert(jread.value().find("Tokyo") != std::string::npos);

    // Invalid inputs -> ToolError::InvalidArgument
    auto empty = writer.execute("");
    assert(!empty.has_value());
    assert(empty.error() == ToolError::InvalidArgument);

    auto missing_content = writer.execute(R"({"filename":"only_name.txt"})");
    assert(!missing_content.has_value());
    assert(missing_content.error() == ToolError::InvalidArgument);

    std::filesystem::remove("result.txt");
    std::filesystem::remove("notes.txt");
    std::filesystem::remove("capital.txt");
    std::filesystem::remove("data.txt");
    std::cout << "  -> PASSED\n";
}

/// W10.75-B-01: Legacy file-tool aliases (file_read, file_write) & end-to-end artifact workflow
void test_file_legacy_aliases_and_artifact_e2e() {
    std::cout << "[TEST] Running test_file_legacy_aliases_and_artifact_e2e...\n";
    ToolRegistry reg;
    reg.register_all_tools();

    // 1. Alias lookup & creation
    Tool* read_tool_alias = reg.lookup("file_read");
    Tool* write_tool_alias = reg.lookup("file_write");
    assert(read_tool_alias != nullptr);
    assert(write_tool_alias != nullptr);
    assert(read_tool_alias->get_name() == "read_file");
    assert(write_tool_alias->get_name() == "write_file");

    auto created_read = reg.create("file_read");
    auto created_write = reg.create("file_write");
    assert(created_read != nullptr);
    assert(created_write != nullptr);
    assert(created_read->get_name() == "read_file");
    assert(created_write->get_name() == "write_file");

    // 2. End-to-end artifact creation via file_write alias with JSON args
    const std::string artifact_path = "artifacts/test_artifact_b01.txt";
    std::filesystem::create_directories("artifacts");

    auto write_res = write_tool_alias->execute(R"({"filename":"artifacts/test_artifact_b01.txt","content":"Header content\n"})");
    assert(write_res.has_value());
    assert(write_res.value().find("wrote") != std::string::npos || write_res.value().find("OK") != std::string::npos);

    // 3. Read back exact content via file_read alias with JSON args
    auto read_res = read_tool_alias->execute(R"({"path":"artifacts/test_artifact_b01.txt"})");
    assert(read_res.has_value());
    assert(read_res.value() == "Header content\n");

    // 4. Append content via append_file tool
    Tool* append_tool = reg.lookup("append_file");
    assert(append_tool != nullptr);
    auto append_res = append_tool->execute(R"({"filename":"artifacts/test_artifact_b01.txt","content":"Appended line\n"})");
    assert(append_res.has_value());

    // 5. Read back and verify both previous and new content exist
    auto read_after_append = read_tool_alias->execute("artifacts/test_artifact_b01.txt");
    assert(read_after_append.has_value());
    assert(read_after_append.value() == "Header content\nAppended line\n");

    // 6. Failure cases matrix for file tools -> ToolError, no exceptions thrown
    // - Unknown legacy name: reg.lookup returns nullptr
    assert(reg.lookup("unknown_file_alias") == nullptr);
    assert(reg.create("unknown_file_alias") == nullptr);

    // - Invalid/missing path for read:
    auto bad_read_path = read_tool_alias->execute("non_existent_file_xyz_123.txt");
    assert(!bad_read_path.has_value());
    assert(bad_read_path.error() == ToolError::NotFound);

    // - Missing content for write:
    auto bad_write_nocontent = write_tool_alias->execute(R"({"filename":"artifacts/test_artifact_b01.txt"})");
    assert(!bad_write_nocontent.has_value());
    assert(bad_write_nocontent.error() == ToolError::InvalidArgument);

    // - Malformed JSON-style argument:
    auto malformed_json_write = write_tool_alias->execute(R"({"filename":"artifacts/test_artifact_b01.txt", "content":)");
    assert(!malformed_json_write.has_value());
    assert(malformed_json_write.error() == ToolError::InvalidArgument);

    // Cleanup temp artifact
    std::filesystem::remove(artifact_path);
    assert(!std::filesystem::exists(artifact_path));

    std::cout << "  -> PASSED\n";
}

/// B-10-02: CalculatorTool trim khoảng trắng và trả ToolError cho input xấu.
void test_calculator_args_trim() {
    std::cout << "[TEST] Running test_calculator_args_trim...\n";
    CalculatorTool calc;

    // Không khoảng trắng
    auto no_space = calc.execute("47*23");
    assert(no_space.has_value());
    assert(no_space.value().find("1081") != std::string::npos);

    // Có khoảng trắng thừa
    auto spaced = calc.execute(" 47 * 23 ");
    assert(spaced.has_value());
    assert(spaced.value().find("1081") != std::string::npos);

    auto add = calc.execute("2+3");
    assert(add.has_value());
    assert(add.value().find("5") != std::string::npos);

    // Invalid -> InvalidArgument; chia cho 0 -> ExecutionFailed
    auto bad = calc.execute("abc");
    assert(!bad.has_value());
    assert(bad.error() == ToolError::InvalidArgument);

    auto div0 = calc.execute("1/0");
    assert(!div0.has_value());
    assert(div0.error() == ToolError::ExecutionFailed);

    std::cout << "  -> PASSED\n";
}

/// B-10-02: MemoryTool hai mode save/search và invalid input trả ToolError.
void test_memory_modes() {
    std::cout << "[TEST] Running test_memory_modes...\n";
    MemoryTool mem;

    auto saved = mem.execute("save Tokyo");
    assert(saved.has_value());

    auto found = mem.execute("search Tokyo");
    assert(found.has_value());
    assert(found.value().find("Tokyo") != std::string::npos);

    auto notfound = mem.execute("search zzz_no_such_keyword_zzz");
    assert(notfound.has_value());
    assert(notfound.value().find("No memory found") != std::string::npos);

    auto bad = mem.execute("unknown_command");
    assert(!bad.has_value());
    assert(bad.error() == ToolError::InvalidArgument);

    auto empty = mem.execute("");
    assert(!empty.has_value());
    assert(empty.error() == ToolError::InvalidArgument);

    // W10.5-B-02: Test custom DB path injection & getter
    MemoryTool custom_mem("custom_test_mem.db", std::make_unique<HashEmbedder>());
    assert(custom_mem.get_db_path() == "custom_test_mem.db");
    auto custom_save = custom_mem.execute("save test item");
    assert(custom_save.has_value());
    std::remove("custom_test_mem.db");

    // W10.5-B-02: Test invalid/unwritable DB path -> returns ToolError::ExecutionFailed safely without crash
    MemoryTool bad_db_mem("/non_existent_dir_xyz_123/bad.db", std::make_unique<HashEmbedder>());
    auto bad_db_save = bad_db_mem.execute("save failing item");
    assert(!bad_db_save.has_value());
    std::cout << "  -> PASSED\n";
}

/// W10.5-B-02: Test MemoryTool lifecycle, is_ready(), and migration.
void test_memory_lifecycle() {
    std::cout << "[TEST] Running test_memory_lifecycle...\n";

    // 1. Normal MemoryTool is_ready() == true
    MemoryTool mem("lifecycle_test.db", std::make_unique<HashEmbedder>());
    assert(mem.is_ready() == true);
    assert(mem.get_db_path() == "lifecycle_test.db");

    // 2. Unwritable path is_ready() == false
    MemoryTool bad_mem("/non_existent_dir_xyz_123/bad.db", std::make_unique<HashEmbedder>());
    assert(bad_mem.is_ready() == false);

    // Executing on unready DB returns ExecutionFailed
    auto res = bad_mem.execute("save item");
    assert(!res.has_value());
    assert(res.error() == ToolError::ExecutionFailed);

    std::remove("lifecycle_test.db");
    std::cout << "  -> PASSED\n";
}

/// B-10-02: ExecTool policy (deny qua registry) và args rỗng trả ToolError.
void test_exec_policy() {
    std::cout << "[TEST] Running test_exec_policy...\n";
    ToolRegistry reg;
    reg.register_all_tools();
    reg.deny("execute_shell");

    // Deny chặn cả alias "exec" trước lookup/create.
    assert(!reg.is_allowed("execute_shell"));
    assert(reg.lookup("exec") == nullptr);
    assert(reg.create("exec") == nullptr);

    ExecTool exec;
    auto empty = exec.execute("");
    assert(!empty.has_value());
    assert(empty.error() == ToolError::InvalidArgument);

    std::cout << "  -> PASSED\n";
}

/// B-10-03: ExecTool timeout offline fixture.
///
///   - Lệnh sleep dài hơn timeout -> ExecutionFailed (không cần mạng).
///   - Lệnh hợp lệ exit 0 -> trả về output.
/// Chạy offline, không phụ thuộc network. Exit-code khác 0 không bị classify
/// thành ToolError vì FunctionalEvaluator dựa trên output nội dung ("PASS")
/// khi script như "test -f missing.txt && echo PASS" trả non-zero.
void test_exec_timeout_offline() {
    std::cout << "[TEST] Running test_exec_timeout_offline...\n";

    // Timeout 200ms, lệnh sleep 2s -> bị kill, trả ExecutionFailed.
    ExecTool fast(std::chrono::milliseconds(200));
    auto timed_out = fast.execute("sleep 2");
    assert(!timed_out.has_value());
    assert(timed_out.error() == ToolError::ExecutionFailed);

    // Lệnh hợp lệ -> có value chứa output.
    ExecTool exec;
    auto ok = exec.execute("printf hello");
    assert(ok.has_value());
    assert(ok.value().find("hello") != std::string::npos);

    std::cout << "  -> PASSED\n";
}

/// B-10-03: WebSearchTool offline fixture.
///
/// Override http_get() (protected virtual seam) để inject:
///   - network failure  -> ExecutionFailed
///   - timeout          -> ExecutionFailed
///   - HTTP error body  -> ExecutionFailed / NotFound theo parse
///   - JSON hợp lệ      -> parse trả AbstractText/Answer
/// Không gửi request thật, chạy ổn định offline.
class FakeWebSearchTool : public WebSearchTool
{
public:
    std::expected<std::string, ToolError> response;

    std::expected<std::string, ToolError>
    http_get(const std::string&) override
    {
        return response;
    }
};

void test_websearch_offline_fixture() {
    std::cout << "[TEST] Running test_websearch_offline_fixture...\n";

    FakeWebSearchTool tool;

    // 1. Timeout / network error -> ExecutionFailed, không throw.
    tool.response = std::unexpected(ToolError::ExecutionFailed);
    auto net = tool.execute("C++ programming");
    assert(!net.has_value());
    assert(net.error() == ToolError::ExecutionFailed);

    // 2. HTTP status failure (500/503/404) -> ExecutionFailed
    tool.response = std::unexpected(ToolError::ExecutionFailed);
    auto http_status_err = tool.execute("weather");
    assert(!http_status_err.has_value());
    assert(http_status_err.error() == ToolError::ExecutionFailed);

    // 3. Malformed JSON response body -> ExecutionFailed
    tool.response = R"({"AbstractText": "truncated...)";
    auto malformed_body = tool.execute("C++");
    assert(!malformed_body.has_value());
    assert(malformed_body.error() == ToolError::ExecutionFailed);

    // 4. HTTP response body with error JSON (no AbstractText/Answer/RelatedTopics) -> NotFound
    tool.response = R"({"error": "internal server error"})";
    auto http_err = tool.execute("C++");
    assert(!http_err.has_value());
    assert(http_err.error() == ToolError::NotFound);

    // 5. JSON hợp lệ: AbstractText -> trả về nội dung.
    tool.response = R"({"AbstractText": "C++ is a compiled language."})";
    auto ok = tool.execute("C++");
    assert(ok.has_value());
    assert(ok.value().find("compiled") != std::string::npos);

    // 6. JSON hợp lệ: chỉ có Answer -> trả về Answer.
    tool.response = R"({"Answer": "42"})";
    auto answer = tool.execute("life");
    assert(answer.has_value());
    assert(answer.value().find("42") != std::string::npos);

    // 7. Args rỗng -> InvalidArgument.
    auto empty = tool.execute("");
    assert(!empty.has_value());
    assert(empty.error() == ToolError::InvalidArgument);

    std::cout << "  -> PASSED\n";
}

/// BNS-V-01: cosine similarity với fixed vectors (deterministic, không mạng).
void test_cosine_similarity_fixed_vectors() {
    std::cout << "[TEST] Running test_cosine_similarity_fixed_vectors...\n";

    // Trùng nhau -> ~1.0
    EmbeddingVector a = {1.0f, 0.0f, 0.0f};
    EmbeddingVector b = {1.0f, 0.0f, 0.0f};
    assert(std::abs(cosine_similarity(a, b) - 1.0f) < 1e-5f);

    // Trực giao -> ~0.0
    EmbeddingVector c = {0.0f, 1.0f, 0.0f};
    assert(std::abs(cosine_similarity(a, c)) < 1e-5f);

    // Ngược chiều -> ~-1.0
    EmbeddingVector d = {-1.0f, 0.0f, 0.0f};
    assert(std::abs(cosine_similarity(a, d) + 1.0f) < 1e-5f);

    // Cùng hướng, khác norm -> ~1.0 (bất biến scale)
    EmbeddingVector e = {3.0f, 0.0f, 0.0f};
    assert(std::abs(cosine_similarity(a, e) - 1.0f) < 1e-5f);

    // Rỗng / lệch size -> 0.0
    assert(cosine_similarity({}, {}) == 0.0f);
    assert(cosine_similarity({1.0f}, {1.0f, 2.0f}) == 0.0f);

    // HashEmbedder deterministic: cùng text -> cùng vector, chiều cố định.
    HashEmbedder he;
    const auto v1 = he.embed("Tokyo weather");
    const auto v2 = he.embed("Tokyo weather");
    const auto v3 = he.embed("Beijing food");
    assert(v1.size() == kEmbeddingDim);
    assert(v1 == v2);
    assert(cosine_similarity(v1, v2) > 0.999f);
    // Text khác nhau về ngữ nghĩa cùng domain vẫn cho similarity hợp lý.
    assert(cosine_similarity(v1, v3) < 1.0f);

    std::cout << "  -> PASSED\n";
}

/// BNS-V-01: integration — lưu memory kèm embedding và ranking theo cosine.
void test_memory_vector_search_ranking() {
    std::cout << "[TEST] Running test_memory_vector_search_ranking...\n";
    MemoryTool mem(std::make_unique<HashEmbedder>());

    // vsave lưu kèm vector; vsearch trả về ranking theo độ tương đồng.
    auto s1 = mem.execute("vsave weather in Tokyo");
    assert(s1.has_value());

    auto s2 = mem.execute("vsave food in Vietnam");
    assert(s2.has_value());

    auto s3 = mem.execute("vsave C++ programming");
    assert(s3.has_value());

    // Query về weather -> kết quả weather đứng đầu.
    auto q1 = mem.execute("vsearch Tokyo climate");
    assert(q1.has_value());
    assert(q1.value().find("weather") != std::string::npos);
    assert(q1.value().find("1.") != std::string::npos);

    // Query về lập trình -> kết quả C++ đứng đầu.
    // (Char n-gram hasher cần lexical overlap; "c++ coding" share "c++/ing"
    // với doc lưu, trong khi weather/food hầu như không có.)
    auto q2 = mem.execute("vsearch c++ coding");
    assert(q2.has_value());
    assert(q2.value().find("C++") != std::string::npos);

    // Invalid: vsave/vsearch rỗng -> InvalidArgument.
    auto bad_save = mem.execute("vsave");
    assert(!bad_save.has_value());
    assert(bad_save.error() == ToolError::InvalidArgument);

    auto bad_search = mem.execute("vsearch");
    assert(!bad_search.has_value());
    assert(bad_search.error() == ToolError::InvalidArgument);

    // save/search cũ vẫn hoạt động (regression).
    auto old_save = mem.execute("save legacy entry");
    assert(old_save.has_value());
    auto old_search = mem.execute("search legacy");
    assert(old_search.has_value());

    std::cout << "  -> PASSED\n";
}

/// BNS-V-01: OllamaEmbedder contract & error handling (no silent hash fallback).
void test_ollama_embedder() {
    std::cout << "[TEST] Running test_ollama_embedder...\n";

    OllamaEmbedder default_emb;
    assert(default_emb.host() == "http://localhost:11434");
    assert(default_emb.model() == "nomic-embed-text");
    assert(default_emb.timeout_seconds() == 10);

    OllamaEmbedder custom_emb("http://localhost:11434", "nomic-embed-text", 5);
    assert(custom_emb.host() == "http://localhost:11434");
    assert(custom_emb.model() == "nomic-embed-text");
    assert(custom_emb.timeout_seconds() == 5);

    // Production MemoryTool defaults to OllamaEmbedder.
    MemoryTool prod_mem;

    // Unreachable host -> vsave/vsearch return ToolError::ExecutionFailed cleanly (no fallback to HashEmbedder).
    MemoryTool offline_ollama_mem(std::make_unique<OllamaEmbedder>("http://127.0.0.1:59999", "nomic-embed-text", 1));
    auto bad_vsave = offline_ollama_mem.execute("vsave test entry");
    assert(!bad_vsave.has_value());
    assert(bad_vsave.error() == ToolError::ExecutionFailed);

    auto bad_vsearch = offline_ollama_mem.execute("vsearch test");
    assert(!bad_vsearch.has_value());
    assert(bad_vsearch.error() == ToolError::ExecutionFailed);

    // save/search (offline regression) work without vector embedder.
    auto legacy_save = offline_ollama_mem.execute("save legacy offline note");
    assert(legacy_save.has_value());

    auto legacy_search = offline_ollama_mem.execute("search offline note");
    assert(legacy_search.has_value());

    std::cout << "  -> PASSED\n";
}

/// W10.5-B-01: OllamaEmbedder empty input and expected_dim tests.
void test_ollama_embedder_validation() {
    std::cout << "[TEST] Running test_ollama_embedder_validation...\n";

    OllamaEmbedder emb("http://localhost:11434", "nomic-embed-text", 5);
    assert(emb.expected_dim() == 0);

    // Direct call with empty string throws std::runtime_error
    try {
        [[maybe_unused]] auto vec = emb.embed("");
        assert(false && "Should have thrown on empty input");
    } catch (const std::runtime_error& e) {
        assert(std::string(e.what()).find("empty input text") != std::string::npos);
    }

    std::cout << "  -> PASSED\n";
}

// C-02: opt-in live acceptance; normal CTest remains offline and reproducible.
void test_live_ollama_vector_acceptance() {
    if (std::getenv("RUN_LIVE_OLLAMA") == nullptr) return;

    std::cout << "[TEST] Running test_live_ollama_vector_acceptance...\n";
    const auto db_path = std::filesystem::path("artifacts") / "live_vector_acceptance.db";
    std::filesystem::create_directories(db_path.parent_path());
    {
        MemoryTool memory(db_path.string(), std::make_unique<OllamaEmbedder>());
        assert(memory.execute("vsave weather forecast for Tokyo").has_value());
        assert(memory.execute("vsave C++ compiler optimization flags").has_value());
        const auto result = memory.execute("vsearch Tokyo weather");
        assert(result.has_value());
        assert(result->find("weather forecast for Tokyo") != std::string::npos);
    }
    std::filesystem::remove(db_path);
    std::cout << "  -> PASSED\n";
}

/// BNS-G-01 (B): ScreenshotTool contract — mock capture, base64 encode.
class FakeScreenshotTool : public ScreenshotTool
{
public:
    std::expected<std::string, ToolError> fake_png;

    std::expected<std::string, ToolError>
    capture_png() override
    {
        return fake_png;
    }
};

void test_screenshot_contract() {
    std::cout << "[TEST] Running test_screenshot_contract...\n";

    // Capture thành công (PNG giả) -> base64 data URI, không throw.
    FakeScreenshotTool ok;
    ok.fake_png = std::string("\x89PNG\r\n\x1a\n fakebytes", 14);
    auto res = ok.execute("");
    assert(res.has_value());
    assert(res.value().find("data:image/png;base64,") == 0);
    // base64 đầy đủ của 14 byte giả = "iVBORw0KGgogZmFrZWI=" (đã verify roundtrip).
    assert(res.value().find("iVBORw0KGgogZmFrZWI=") != std::string::npos);

    // Capture thất bại -> NotFound, không throw/crash.
    FakeScreenshotTool fail;
    fail.fake_png = std::unexpected(ToolError::NotFound);
    auto err = fail.execute("hint");
    assert(!err.has_value());
    assert(err.error() == ToolError::NotFound);

    // base64_encode padding 1 byte và 2 byte đúng chuẩn.
    assert(ScreenshotTool::base64_encode("M") == "TQ==");
    assert(ScreenshotTool::base64_encode("Ma") == "TWE=");
    assert(ScreenshotTool::base64_encode("Man") == "TWFu");

    std::cout << "  -> PASSED\n";
}

/// BNS-G-01 (B): ActionTool — allow-list, validate input, deny không hợp lệ.
class RecordingActionTool : public ActionTool
{
public:
    bool performed = false;
    std::string last_action;
    std::string last_payload;

    std::expected<std::string, ToolError>
    perform_action(const std::string& action,
                   const std::string& payload) override
    {
        performed = true;
        last_action = action;
        last_payload = payload;
        return std::string("ok");
    }
};

void test_action_tool_safety() {
    std::cout << "[TEST] Running test_action_tool_safety...\n";

    // click hợp lệ -> perform_action được gọi, payload đúng.
    RecordingActionTool click;
    auto c = click.execute("click 100 200");
    assert(c.has_value());
    assert(click.performed);
    assert(click.last_action == "click");
    assert(click.last_payload == "100 200");

    // type_text hợp lệ -> payload là text.
    RecordingActionTool type;
    auto t = type.execute("type_text hello world");
    assert(t.has_value());
    assert(type.last_action == "type_text");
    assert(type.last_payload == "hello world");

    // key_press với key trong allow-list -> pass.
    RecordingActionTool key;
    auto k = key.execute("key_press return");
    assert(k.has_value());
    assert(key.last_action == "key_press");
    assert(key.last_payload == "return");

    // Hành động ngoài allow-list -> AccessDenied.
    RecordingActionTool denied;
    auto d = denied.execute("run_any_shell rm -rf /");
    assert(!d.has_value());
    assert(d.error() == ToolError::AccessDenied);
    assert(!denied.performed);

    // Key ngoài allow-list -> AccessDenied.
    auto bad_key = denied.execute("key_press ctrl+c");
    assert(!bad_key.has_value());
    assert(bad_key.error() == ToolError::AccessDenied);

    // Toạ độ âm / quá lớn -> InvalidArgument.
    auto neg = denied.execute("click -5 10");
    assert(!neg.has_value());
    assert(neg.error() == ToolError::InvalidArgument);

    auto huge = denied.execute("click 1000001 0");
    assert(!huge.has_value());
    assert(huge.error() == ToolError::InvalidArgument);

    // Args rỗng / thiếu thành phần -> InvalidArgument.
    auto empty = denied.execute("");
    assert(!empty.has_value());
    assert(empty.error() == ToolError::InvalidArgument);

    auto no_xy = denied.execute("click 10");
    assert(!no_xy.has_value());
    assert(no_xy.error() == ToolError::InvalidArgument);

    // type_text quá dài -> InvalidArgument.
    std::string long_text(ActionTool::kMaxTextLength + 1, 'a');
    auto too_long = denied.execute("type_text " + long_text);
    assert(!too_long.has_value());
    assert(too_long.error() == ToolError::InvalidArgument);

    std::cout << "  -> PASSED\n";
}

int main() {
    std::cout << "=== RUNNING ROLE B TOOL REGISTRY & FACTORY FOCUSED TESTS ===\n";
    test_registry_instance_registration();
    test_factory_creation();
    test_aliases_and_normalization();
    test_allow_deny_policies();
    test_duplicate_creator_overwrite();
    test_register_all_tools();
    test_tool_error_paths();
    test_canonical_names_and_descriptions();
    test_file_args_formats();
    test_file_legacy_aliases_and_artifact_e2e();
    test_calculator_args_trim();
    test_memory_modes();
    test_memory_lifecycle();
    test_exec_policy();
    test_exec_timeout_offline();
    test_websearch_offline_fixture();
    test_cosine_similarity_fixed_vectors();
    test_memory_vector_search_ranking();
    test_ollama_embedder();
    test_ollama_embedder_validation();
    test_live_ollama_vector_acceptance();
    test_screenshot_contract();
    test_action_tool_safety();
    std::cout << "=== ALL ROLE B TOOL TESTS PASSED SUCCESSFULLY ===\n";
    return 0;
}
