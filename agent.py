"""Research Agent - Standalone script for LangGraph deployment.

This module creates a deep research agent with custom tools and prompts
for conducting web research with strategic thinking and context management.
"""

from datetime import datetime

# from langchain.chat_models import init_chat_model
from deepagents import create_deep_agent
from deepagents.backends import CompositeBackend, StateBackend, StoreBackend
from langchain_core.runnables import RunnableConfig
from langchain_google_genai import ChatGoogleGenerativeAI
from langfuse.langchain import CallbackHandler

from research_agent.prompts import researcher_raw_prompt, supervisor_raw_prompt
from research_agent.tools import tavily_search, think_tool

# Limits
max_concurrent_research_units = 3
max_researcher_iterations = 3

# Get current date
current_date = datetime.now().strftime("%Y-%m-%d")
langfuse_handler = CallbackHandler()

# Create research sub-agent
research_sub_agent = {
    "name": "research-agent",
    "description": "Delegate research to the sub-agent researcher. Only give this researcher one topic at a time.",
    "system_prompt": researcher_raw_prompt.compile(date=current_date),
    "tools": [tavily_search, think_tool],
}

model = ChatGoogleGenerativeAI(model="gemini-3-pro-preview")

# model = init_chat_model(model="anthropic:claude-sonnet-4-5-20250929")

config = RunnableConfig(callbacks=[langfuse_handler])


def _make_backend(runtime):
    return CompositeBackend(
        default=StateBackend(runtime),  # Ephemeral storage
        routes={
            "/memories/": StoreBackend(runtime)  # Persistent storage
        },
    )


# Create the agent
agent = create_deep_agent(
    model=model,
    tools=[think_tool],
    system_prompt=supervisor_raw_prompt.compile(
        max_concurrent_research_units=max_concurrent_research_units,
        max_researcher_iterations=max_researcher_iterations,
    ),
    subagents=[research_sub_agent],
    debug=True,
    backend=_make_backend,
).with_config(config)
