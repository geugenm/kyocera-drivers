# Vendored agent skills — refresh with:
#   cmake -P .agents/install_skills.cmake
# Browse more: https://www.skills.sh/
cmake_minimum_required(VERSION 3.31)

find_program(node_executable NAMES node nodejs REQUIRED)
find_program(npx_executable NAMES npx npx.cmd REQUIRED)
find_program(git_executable NAMES git REQUIRED)

execute_process(
    COMMAND ${git_executable} rev-parse --show-toplevel
    OUTPUT_VARIABLE git_root
    OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY)

if(WIN32)
    set(npx_invoke cmd /c ${npx_executable})
else()
    set(npx_invoke ${npx_executable})
endif()

function(install_agent_skills)
    cmake_parse_arguments(PARSE_ARGV 0 arg "" "URL" "SKILLS")
    if(NOT arg_URL)
        message(FATAL_ERROR "install_agent_skills: URL missing")
    endif()
    set(skill_flags "")
    foreach(s IN LISTS arg_SKILLS)
        list(APPEND skill_flags --skill ${s})
    endforeach()
    execute_process(
        COMMAND
            ${CMAKE_COMMAND} -E env npm_config_package_lock=false CI=true
            NO_COLOR=1 TERM=dumb
            ${npx_invoke} --yes skills add ${arg_URL} -a opencode -y
            ${skill_flags}
        WORKING_DIRECTORY "${git_root}"
        TIMEOUT 600
        COMMAND_ECHO STDOUT
        COMMAND_ERROR_IS_FATAL ANY)
endfunction()

install_agent_skills(
    URL
    https://github.com/vercel-labs/skills
    SKILLS
    find-skills)
install_agent_skills(
    URL
    https://github.com/mattpocock/skills
    SKILLS
    code-review
    research)
install_agent_skills(
    URL
    https://github.com/github/awesome-copilot
    SKILLS
    git-commit)

# Keep the lock file next to the rest of the agent workflow.
file(MAKE_DIRECTORY "${git_root}/.agents")
if(EXISTS "${git_root}/skills-lock.json")
    file(RENAME "${git_root}/skills-lock.json"
         "${git_root}/.agents/skills-lock.json")
endif()
