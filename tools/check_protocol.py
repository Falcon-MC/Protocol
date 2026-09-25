import argparse
import json
import re
import sys
from pathlib import Path

PUT_TOKENS = {
    "put": ["raw"],
    "putByte": ["u8"],
    "putBool": ["bool"],
    "putOptionalPresent": ["bool"],
    "putShort": ["i16be"],
    "putLShort": ["i16le"],
    "putInt": ["i32be"],
    "putLInt": ["i32le"],
    "putLong": ["i64be"],
    "putLLong": ["i64le"],
    "putFloat": ["f32be"],
    "putLFloat": ["f32le"],
    "putDouble": ["f64be"],
    "putLDouble": ["f64le"],
    "putUnsignedVarInt": ["uvar32"],
    "putArrayLength": ["uvar32"],
    "putVarInt": ["var32"],
    "putUnsignedVarLong": ["uvar64"],
    "putVarLong": ["var64"],
    "putString": ["string"],
    "putByteArray": ["string"],
    "putVector3f": ["f32le", "f32le", "f32le"],
    "putVector2f": ["f32le", "f32le"],
    "putVector3i": ["var32", "var32", "var32"],
    "putBlockPosition": ["var32", "var32", "var32"],
    "putUuid": ["i64le", "i64le"],
}

COMPATIBLE = {("bool", "u8"), ("u8", "bool")}
OPEN = "["
CLOSE = "]"
OPAQUE = "opaque"
STOP = "stop"

STATEMENT = re.compile(
    r"\b(for|while|if|switch|else)\b"
    r"|\bstream\s*\.\s*(put\w*)\s*\("
    r"|\b([A-Za-z_]\w*(?:::[A-Za-z_]\w*)*)\s*\(\s*stream\b"
    r"|([A-Za-z_][\w.\->\[\]]*?)\s*(?:\.|->)\s*(write\w*)\s*\(\s*stream\b"
)
DEFINITION = re.compile(r"^[\w:<>,\s&*]*?\b([A-Za-z_]\w*(?:::[A-Za-z_]\w*)*)\s*\(([^;{]*)\)\s*(?:const\s*)?\{",
                        re.MULTILINE)


class Token:
    def __init__(self, kind, field="", depth=0, detail=""):
        self.kind = kind
        self.field = field
        self.depth = depth
        self.detail = detail
        self.starts_field = False

    def label(self):
        if self.kind == OPAQUE:
            return f"call {self.detail}"
        if self.kind == STOP:
            return f"unchecked ({self.detail})"
        return self.kind


def strip_comments(text):
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.DOTALL)
    return re.sub(r"//[^\n]*", "", text)


def matching(text, start, opening, closing):
    depth = 0
    index = start
    quote = None
    while index < len(text):
        character = text[index]
        if quote:
            if character == "\\":
                index += 2
                continue
            if character == quote:
                quote = None
        elif character in "\"'":
            quote = character
        elif character == opening:
            depth += 1
        elif character == closing:
            depth -= 1
            if depth == 0:
                return index
        index += 1
    return len(text) - 1


def statement_end(text, start):
    index = start
    depth = 0
    while index < len(text):
        character = text[index]
        if character in "({[":
            depth += 1
        elif character in ")}]":
            depth -= 1
        elif character == ";" and depth == 0:
            return index
        index += 1
    return len(text) - 1


def body_after(text, start):
    index = start
    while index < len(text) and text[index].isspace():
        index += 1
    if index < len(text) and text[index] == "{":
        end = matching(text, index, "{", "}")
        return text[index + 1:end], end + 1
    end = statement_end(text, index)
    return text[index:end + 1], end + 1


class FalconIndex:
    def __init__(self, root):
        self.functions = {}
        self.local = {}
        for path in sorted(root.rglob("*.cpp")):
            text = strip_comments(path.read_text(encoding="utf-8", errors="replace"))
            self.local[path] = {}
            for match in DEFINITION.finditer(text):
                name = match.group(1)
                if "stream" not in match.group(2):
                    continue
                open_brace = text.index("{", match.end() - 1)
                body = text[open_brace + 1:matching(text, open_brace, "{", "}")]
                self.local[path][name.split("::")[-1]] = body
                if "::" in name:
                    self.functions[name] = (path, body)

    def resolve(self, path, name):
        if name in self.functions:
            return self.functions[name]
        short = name.split("::")[-1]
        if "::" not in name and short in self.local.get(path, {}):
            return path, self.local[path][short]
        return None


def falcon_tokens(index, path, code, depth=0, call_depth=0):
    tokens = []
    position = 0
    while True:
        match = STATEMENT.search(code, position)
        if not match:
            return tokens

        keyword, put, call, member, member_call = match.groups()
        if keyword in ("for", "while"):
            header_start = code.index("(", match.end())
            header_end = matching(code, header_start, "(", ")")
            body, position = body_after(code, header_end + 1)
            inner = falcon_tokens(index, path, body, depth + 1, call_depth)
            if inner:
                tokens.append(Token(OPEN, depth=depth))
                tokens.extend(inner)
                tokens.append(Token(CLOSE, depth=depth))
        elif keyword == "if":
            header_start = code.index("(", match.end())
            header_end = matching(code, header_start, "(", ")")
            body, position = body_after(code, header_end + 1)
            if not re.search(r"\breturn\b", body):
                tokens.extend(falcon_tokens(index, path, body, depth, call_depth))
            while True:
                rest = re.match(r"\s*else\b", code[position:])
                if not rest:
                    break
                position += rest.end()
                chained = re.match(r"\s*if\s*\(", code[position:])
                if chained:
                    header_start = position + chained.end() - 1
                    header_end = matching(code, header_start, "(", ")")
                    _, position = body_after(code, header_end + 1)
                else:
                    _, position = body_after(code, position)
        elif keyword == "switch":
            header_start = code.index("(", match.end())
            header_end = matching(code, header_start, "(", ")")
            _, position = body_after(code, header_end + 1)
            tokens.append(Token(STOP, depth=depth, detail="switch"))
        elif keyword == "else":
            _, position = body_after(code, match.end())
        elif put:
            open_paren = match.end() - 1
            position = matching(code, open_paren, "(", ")") + 1
            for kind in PUT_TOKENS.get(put, []):
                tokens.append(Token(kind, depth=depth))
            if put not in PUT_TOKENS:
                tokens.append(Token(OPAQUE, depth=depth, detail=f"stream.{put}"))
        elif call:
            open_paren = code.index("(", match.start())
            position = matching(code, open_paren, "(", ")") + 1
            resolved = index.resolve(path, call) if call_depth < 8 else None
            if resolved:
                tokens.extend(falcon_tokens(index, resolved[0], resolved[1], depth, call_depth + 1))
            else:
                tokens.append(Token(OPAQUE, depth=depth, detail=call))
        else:
            open_paren = code.index("(", match.start() + len(member))
            position = matching(code, open_paren, "(", ")") + 1
            tokens.append(Token(OPAQUE, depth=depth, detail=f"{member}.{member_call}"))


class MojangDocs:
    def __init__(self, root):
        self.root = root
        self.cache = {}

    def load(self, name):
        if name not in self.cache:
            path = self.root / name
            self.cache[name] = json.loads(path.read_text(encoding="utf-8")) if path.exists() else None
        return self.cache[name]

    def packets(self):
        result = {}
        for path in sorted(self.root.glob("*Packet.json")):
            schema = self.load(path.name)
            meta = (schema or {}).get("$metaProperties", {})
            if "[cereal:packet]" in meta:
                result[schema["title"]] = (meta["[cereal:packet]"], schema)
        return result

    def protocol(self):
        for schema in self.packets().values():
            return schema[1].get("x-protocol-version")
        return None


def integer_token(underlying, options):
    width = {"int8": 8, "uint8": 8, "int16": 16, "uint16": 16, "int32": 32, "uint32": 32,
             "int64": 64, "uint64": 64}.get(underlying)
    if width is None:
        return None
    if width == 8:
        return "cbyte" if underlying == "uint8" and "Compression" in options else "u8"
    if "Compression" in options:
        prefix = "var" if underlying.startswith("int") else "uvar"
        return f"{prefix}{64 if width == 64 else 32}"
    return f"i{width}{'be' if 'Big Endian' in options else 'le'}"


def primitive_token(schema):
    underlying = schema.get("x-underlying-type")
    options = schema.get("x-serialization-options", [])
    if "enum" in schema:
        if "Enum-as-Value" not in options:
            return "string"
        return integer_token(underlying, options) if underlying else None
    if underlying == "bool" or schema.get("type") == "boolean":
        return "bool"
    if underlying in ("float", "double"):
        return f"f{32 if underlying == 'float' else 64}{'be' if 'Big Endian' in options else 'le'}"
    if underlying:
        return integer_token(underlying, options)
    if schema.get("type") == "integer" and "Compression" in options:
        return "var32"
    if schema.get("type") == "string" and "enum" not in schema:
        return "string"
    return None


def mojang_tokens(docs, schema, field, depth, seen):
    if schema is None:
        return [Token(STOP, field, depth, "missing schema")]

    if "oneOf" in schema or "anyOf" in schema:
        control = schema.get("x-control-value-type", "?")
        return [Token(STOP, field, depth, f"variant switched on {control}")]

    if "$ref" not in schema:
        token = primitive_token(schema)
        if token:
            return [Token(token, field, depth)]

    if "$ref" in schema:
        name = schema["$ref"].split("/")[-1]
        if name in seen:
            return [Token(STOP, field, depth, f"recursive {name}")]
        target = docs.load(name)
        if target is not None:
            target = dict(target)
            for key in ("x-underlying-type", "x-serialization-options"):
                if key in schema and key not in target:
                    target[key] = schema[key]
        return mojang_tokens(docs, target, field, depth, seen | {name})

    kind = schema.get("type")
    if kind == "array":
        options = schema.get("x-serialization-options", [])
        if "No size compression" in options:
            length = "i32be" if "Big Endian" in options else "i32le"
        else:
            length = "uvar32"
        item_schema = dict(schema.get("items", {}))
        inherited = [option for option in options if option != "No size compression"]
        if inherited and "x-serialization-options" not in item_schema:
            item_schema["x-serialization-options"] = inherited
        items = mojang_tokens(docs, item_schema, field, depth + 1, seen)
        return [Token(length, field, depth), Token(OPEN, field, depth)] + items + [Token(CLOSE, field, depth)]

    if kind == "object":
        if "additionalProperties" in schema and not schema.get("properties"):
            return [Token(STOP, field, depth, "map")]
        properties = schema.get("properties", {})
        required = set(schema.get("required", properties.keys()))
        ordered = sorted(properties.items(), key=lambda item: item[1].get("x-ordinal-index", 0))
        tokens = []
        for name, child in ordered:
            path = f"{field}.{name}" if field else name
            start = len(tokens)
            if name not in required:
                tokens.append(Token("bool", path, depth, "optional presence"))
            tokens.extend(mojang_tokens(docs, child, path, depth, seen))
            if start < len(tokens):
                tokens[start].starts_field = True
        return tokens

    if kind == "null":
        return []

    if kind is None:
        return [Token(STOP, field, depth, "untyped data such as NBT")]

    return [Token(STOP, field, depth, f"unsupported {kind}")]


def top_field(token):
    return token.field.split(".")[0] if token.field else ""


def same(want, have):
    if want.kind == "cbyte":
        return have.kind in ("u8", "uvar32", "bool")
    return want.kind == have.kind or (want.kind, have.kind) in COMPATIBLE


def compare(expected, actual, notes):
    left = 0
    right = 0
    verified = set()
    helpers = set()
    while left < len(expected) and right < len(actual):
        want = expected[left]
        have = actual[right]

        if want.kind == STOP:
            return "partial", f"stopped at `{want.field}`: {want.detail}", verified, helpers
        if have.kind == STOP:
            return "partial", f"stopped at `{want.field}`: Falcon uses a {have.detail}", verified, helpers

        if have.kind == OPAQUE:
            helpers.add(have.detail)
            depth = have.depth
            field = top_field(want)
            left += 1
            while left < len(expected):
                following = expected[left]
                if following.kind == CLOSE and following.depth < depth:
                    break
                if following.depth <= depth and following.kind != CLOSE and top_field(following) != field:
                    break
                left += 1
            right += 1
            continue

        following = actual[right + 1] if right + 1 < len(actual) else None
        presence = have.kind == "bool" and want.starts_field and want.kind != "bool" and following is not None \
            and following.kind != "bool" and (following.kind == OPAQUE or same(want, following))
        if presence:
            notes["optional"].add(f"`{want.field}`")
            right += 1
            continue

        if same(want, have):
            if want.kind not in (OPEN, CLOSE):
                verified.add(top_field(want))
            if want.kind == "cbyte":
                notes["cbyte"].add(f"`{want.field}` as {have.kind}")
            left += 1
            right += 1
            continue

        return "mismatch", f"`{want.field or '?'}`: expected {want.label()}, Falcon writes {have.label()}", \
            verified, helpers

    if left < len(expected):
        remaining = expected[left]
        if remaining.kind == STOP:
            return "partial", f"stopped at `{remaining.field}`: {remaining.detail}", verified, helpers
        return "mismatch", f"`{remaining.field}` ({remaining.label()}) is never written", verified, helpers
    if right < len(actual):
        return "mismatch", f"Falcon writes an extra {actual[right].label()} after the last field", verified, helpers
    return "match", "", verified, helpers


def packet_ids(header):
    text = strip_comments(header.read_text(encoding="utf-8"))
    return {name: int(value) for name, value in re.findall(r"\b(\w+)\s*=\s*(\d+)\s*,", text)}


def falcon_protocol(root):
    info = root / "include" / "Protocol" / "ProtocolInfo.h"
    match = re.search(r"CURRENT_PROTOCOL\s*=\s*(\d+)", info.read_text(encoding="utf-8"))
    return int(match.group(1)) if match else None


def merge_length_prefixed(tokens):
    merged = []
    for token in tokens:
        if token.kind == "raw" and merged and merged[-1].kind == "uvar32":
            starts = merged[-1].starts_field
            merged[-1] = Token("string", merged[-1].field, merged[-1].depth)
            merged[-1].starts_field = starts
            continue
        merged.append(token)
        kinds = [entry.kind for entry in merged[-4:]]
        if len(kinds) == 4 and kinds[:2] == ["uvar32", OPEN] and kinds[2] in ("u8", "cbyte") and kinds[3] == CLOSE:
            head = merged[-4]
            blob = Token("string", head.field, head.depth)
            blob.starts_field = head.starts_field
            del merged[-4:]
            merged.append(blob)
    return merged


def falcon_packets(root, index):
    packets = {}
    for path in sorted((root / "src" / "Protocol" / "Packets").glob("*.cpp")):
        text = strip_comments(path.read_text(encoding="utf-8", errors="replace"))
        match = re.search(r"void\s+(\w+)::write\s*\(\s*BinaryStream\s*&\s*stream", text)
        if not match:
            continue
        open_brace = text.index("{", match.end())
        body = text[open_brace + 1:matching(text, open_brace, "{", "}")]
        packets[match.group(1)] = merge_length_prefixed(falcon_tokens(index, path, body))
    return packets


def main():
    parser = argparse.ArgumentParser(description="Compare Falcon's packet encoders with the published schemas")
    parser.add_argument("--protocol", type=Path, default=Path(__file__).resolve().parent.parent)
    parser.add_argument("--docs", type=Path, required=True, help="the json folder of the protocol documentation")
    parser.add_argument("--report", type=Path, default=Path("protocol-report.md"))
    parser.add_argument("--strict", action="store_true", help="exit with 1 when a packet mismatches")
    parser.add_argument("--explain", metavar="PACKET", help="print both encodings of one packet side by side")
    arguments = parser.parse_args()

    docs = MojangDocs(arguments.docs)
    index = FalconIndex(arguments.protocol / "src")
    documented = docs.packets()
    implemented = falcon_packets(arguments.protocol, index)

    if arguments.explain:
        expected = merge_length_prefixed(mojang_tokens(docs, documented[arguments.explain][1], "", 0, set()))
        actual = implemented.get(arguments.explain, [])
        for position in range(max(len(expected), len(actual))):
            want = expected[position] if position < len(expected) else None
            have = actual[position] if position < len(actual) else None
            left = f"{want.label()} {want.field}" if want else ""
            right = have.label() if have else ""
            print(f"{position:4} {left[:90]:90} | {right}")
        return 0
    ids = packet_ids(arguments.protocol / "include" / "Protocol" / "MinecraftPacketIds.h")

    results = []
    optional_notes = []
    byte_notes = []
    for name, tokens in sorted(implemented.items()):
        if name not in documented:
            continue
        expected = merge_length_prefixed(mojang_tokens(docs, documented[name][1], "", 0, set()))
        notes = {"optional": set(), "cbyte": set()}
        status, detail, verified, helpers = compare(expected, tokens, notes)
        results.append((name, status, detail, verified, helpers))
        optional_notes.extend(f"{name}: {field}" for field in sorted(notes["optional"]))
        byte_notes.extend(f"{name}: {field}" for field in sorted(notes["cbyte"]))

    id_mismatches = []
    for name, (packet_id, _) in documented.items():
        short = name[:-len("Packet")]
        if short in ids and ids[short] != packet_id:
            id_mismatches.append((name, ids[short], packet_id))

    missing = sorted(name for name in documented if name not in implemented)
    undocumented = sorted(name for name in implemented if name not in documented)
    counts = {status: sum(1 for result in results if result[1] == status)
              for status in ("match", "partial", "mismatch")}

    lines = ["# Protocol check", ""]
    lines.append(f"- Falcon protocol: **{falcon_protocol(arguments.protocol)}**, documentation protocol: "
                 f"**{docs.protocol()}**")
    lines.append(f"- {len(results)} packets compared: **{counts['match']} match**, {counts['partial']} partially "
                 f"checked, **{counts['mismatch']} mismatch**")
    lines.append(f"- {len(id_mismatches)} packet id differences, {len(missing)} documented packets not implemented, "
                 f"{len(undocumented)} implemented packets not documented")
    lines.append("")

    if id_mismatches:
        lines += ["## Packet id differences", "", "| Packet | Falcon | Documentation |", "|---|---|---|"]
        lines += [f"| {name} | {falcon} | {documented_id} |" for name, falcon, documented_id in id_mismatches]
        lines.append("")

    mismatches = [result for result in results if result[1] == "mismatch"]
    if mismatches:
        lines += ["## Mismatches", "", "| Packet | First difference |", "|---|---|"]
        lines += [f"| {name} | {detail} |" for name, _, detail, _, _ in mismatches]
        lines.append("")

    partial = [result for result in results if result[1] == "partial"]
    if partial:
        lines += ["## Partially checked", "", "| Packet | Fields verified | Reason |", "|---|---|---|"]
        lines += [f"| {name} | {len(verified)} | {detail} |" for name, _, detail, verified, _ in partial]
        lines.append("")

    if missing:
        lines += ["## Documented packets not implemented", "", ", ".join(missing), ""]
    if undocumented:
        lines += ["## Implemented packets not in the documentation", "", ", ".join(undocumented), ""]

    if optional_notes:
        lines += ["## Optional fields the documentation lists as required", "",
                  "Falcon writes a presence flag before these fields. Check them against a newer documentation.", ""]
        lines += [f"- {note}" for note in optional_notes]
        lines.append("")

    if byte_notes:
        lines += ["## Compressed bytes", "",
                  "The documentation marks these 8-bit values as compressed. A byte and an unsigned varint are the "
                  "same bytes below 128, so both are accepted.", ""]
        lines += [f"- {note}" for note in byte_notes]
        lines.append("")

    matched = [result for result in results if result[1] == "match"]
    if matched:
        lines += ["## Matching packets", "", ", ".join(name for name, *_ in matched), ""]

    report = "\n".join(lines)
    arguments.report.write_text(report, encoding="utf-8")
    print(report)

    if arguments.strict and (mismatches or id_mismatches):
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
