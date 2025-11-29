# Writing a Story

Writing a story for `ttyny` is as simple as creating a JSON file that adheres 
to the schema defined in 
[`tools/story.schema.json`](../tools/story.schema.json).

The schema is used both as a validation tool and as the sole source of truth
for the documentation.

## Why JSON?

I am not a good story writer. The examples have been created with the help of
LLMs, which tend to produce better outputs in structured formats they have 
extensively been trained on, like JSON.

## GUI

If you prefer viewing the schema in a GUI, you can use
[this tool](https://github.com/atlassian-labs/json-schema-viewer) to preview
the schema.

[This link](https://json-schema.app/view/%23?url=https%3A%2F%2Fraw.githubusercontent.com%2Fshikaan%2Fttyny%2Frefs%2Fheads%2Fmain%2Ftools%2Fstory.schema.json)
takes you directly to the latest version of the schema.

> [!NOTE]
> I am not affiliated with Atlassian in any way. This is just a tool I found
> on the internet.

## Validation

The [`validate`](../tools/validate) tool can be used to validate your schema.
It can be helpful for detecting errors in the story before feeding it to `ttyny`.

It requires [`bun`](https://bun.sh) to run, and can be used like this:

```sh
# in the root directory of this repo
./tools/validate <path-to-story.json>
```

For example:

```sh
./tools/validate assets/psyche.json
# > Validating assets/psyche.json...
# > Validating assets/psyche.json... OK
```

## Usage with LLMs

Using a structured format (JSON) and having a schema makes it very easy to
generate stories with Large Language Models.

Most models these days can take the whole schema as input and generate
coherent and functional stories with just a couple of prompts.
