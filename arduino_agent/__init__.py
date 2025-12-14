"""Deep Research Agent Example.

This module demonstrates building a research agent using the deepagents package
with custom tools for web search and strategic thinking.
"""

from arduino_agent.tools import (
    display_countdown,
    display_text,
    play_sound,
    start_led_blinker,
)

__all__ = ["display_countdown", "display_text", "play_sound", "start_led_blinker"]
