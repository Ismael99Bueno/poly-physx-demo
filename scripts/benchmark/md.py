def write_markdown_recursive(summary: dict, content: str = "", depth: int = 1) -> str:
    for key, value in summary.items():
        if isinstance(value, dict):
            content += f"{'#' * depth} {key}\n"
            content = write_markdown_recursive(value, content, depth + 1)
        else:
            content += f"- {key}: {value}\n"
    return content
