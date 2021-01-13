# Porting Ops from Lite to Micro

This is a guide to porting ops from lite to micro. It begins by explaining,
step-by-step, the necessary code changes and the process of submitting them for
review and acceptance by the project. 

## Step-by-step Guide

The following steps outline the process of porting an op. The process results
in several pull requests (PRs) for ease of review and acceptance.

1.  Search for GitHub issues titled with the name of the op to ensure a port
    isn't already in progress.

1.  Open a GitHub issue to announce your intent and begin the record of your
    work.

    Document the entire process of porrting the op in this issue. Link
    constituent PRs (more below) to this issue. See [this article][Providing
    Context] for background on documenting your work via bug reports.

    A good prototype is issue [#45306][], *micro: port op FILL from lite*. 

1.  **PR-1**: Extract the code for parsing op parameters out of 
    [ParseOpDataTfLite()][] in lite/core/api/flatbuffer_conversions.cc.

    Extract the parsing of op parameters out of the switch statement in
    [ParseOpDataTfLite()][] in *lite/core/api/flatbuffer_conversions.cc*, into
    a standalone function which can be called by the micro op resolver in a
    later PR.
    
    A simple example is PR [#45307][], and a more complicated example is PR
    [#46021][].

    Run the lite test suite to test this change:

        bazel test tensorflow/lite/kernels:all

1.  **PR-2**: Extract the reference for the op in [reference_ops.h][] to a
    standalone header.

    Move the reference implementation of the op to a standalone header so that
    micro can include it without including unrelated dependencies via
    reference_ops.h.

    A good example is PR [#45311][].

    1.  Copy an existing header from
        *tensorflow/lite/kernels/internal/reference/* to
        *tensorflow/lite/kernels/internal/reference/<new_op.h>* to create the
        boilerplate.

    1.  Move the implementation from
        *tensorflow/lite/kernels/internal/reference/reference_ops.h* to
        *tensorflow/lite/kernels/internal/reference/<new_op.h>*.

    1.  Add the new header to the build in
        *tensorflow/lite/kernels/internal/BUILD* under *reference_base* and
        *legacy_reference_base*. E.g., [for FILL][].

        [for FILL]: https://github.com/tensorflow/tensorflow/pull/45311/commits/92f459e6b917fa5099ef5317d14c5100d33a86f0#diff-0b0fc9e1affece3c5a141ee9326f882876b6b958bc8b12a7c01d7540dc04983e

    Do not clang-format existing code in *BUILD* or *reference_ops.h*, nor
    modify it to satisfy cpplint.py.

    Run the lite test suite to test this change:

        bazel test tensorflow/lite/kernels:all

1.  **PR-3: Copy the kernel and test from lite to micro without making any
    modifications and without adding them to the build.**
    
    A good example is PR [#45457][]. This copy action is its own PR in order to
    create readable, reviewable diffs when modifications are made in later PRs.
    If the files were copied and modified in one step, the modifications would
    not appear as a diff of the lite version. Instead, the files would simply
    appear at the destination path in their final form.

    Even this copied, inactive code must pass CI's clang-format check prior to
    merge, and unfortunately the version from lite is not always already
    clang-formatted. Do clang-format it before submitting. This exception to
    the copy-without-modifying directive is unavoidable and allowed.

1.  **PR-4: Remove the bulk of lite-specific code from the files copied to
    micro in the previous step.**
    
    A good example is PR [#45646][]. This bulk-delete action is its own PR for
    reasons similar to those given for PR-3 above: to produce a more readable,
    reviewable diff in this step and in the next. Because the files are not yet
    added to the build, they need not (and obviously won't) compiler or
    function. What to delete now as opposed to deleting in the next PR is
    somewhat subjective, but make deletes in order to:
    
    -   Flatten the namespace down to `tflite`.
    -   Stop resizing output tensors.
    -   Remove input and output types other than int8 and float32.
    -   Stop using gtest.
    -   etc.

1.  **PR-5: Make the necessary changes to the micro kernel, header, and test to
    make the op implementation suitable for micro. Include these in the
    build.**

    This step requires the most creativity, and may receive the most feedback
    during review. Maintain good atomicity in your commits; considering its
    scope, this PR should contain more than one commit. A good example is PR
    [#45647][]. 

    Refer to the [general porting guidelines][] below.

    Run the micro test suites to test this change:

        bazel test tensorflow/lite/micro/kernels:all
        bazel test tensorflow/lite/micro/kernels:${op}_test
        make -f tensorflow/lite/micro/tools/make/Makefile test
        make -f tensorflow/lite/micro/tools/make/Makefile test_kernel_${op}_test

[#45306]: https://github.com/tensorflow/tensorflow/issues/45306
[#45307]: https://github.com/tensorflow/tensorflow/pull/45307
[#45311]: https://github.com/tensorflow/tensorflow/pull/45311
[#45457]: https://github.com/tensorflow/tensorflow/pull/45457
[#45646]: https://github.com/tensorflow/tensorflow/pull/45646
[#45647]: https://github.com/tensorflow/tensorflow/pull/45647
[#46021]: https://github.com/tensorflow/tensorflow/pull/46021
[Providing Context]: https://testing.googleblog.com/2017/09/code-health-providing-context-with.html
[ParseOpDataTfLite()]: https://github.com/tensorflow/tensorflow/blob/d8394a6d774f5e3c02d97f1fc18ff445199db598/tensorflow/lite/core/api/flatbuffer_conversions.cc#L135
[reference_ops.h]: https://github.com/tensorflow/tensorflow/blob/92f459e6b917fa5099ef5317d14c5100d33a86f0/tensorflow/lite/kernels/internal/reference/reference_ops.h
[general porting guidelines]: #general-porting-guidelines


### Notes

*   There was discussion of commits vs. PRs in [#45387][].

*   On Debian, running bazel required installing package bazel-3.1.0.

*   If you have permission, add the label *comp:micro* to these PRs.

*   If you have permission, the label *kokoro:force-run* can be applied to
    manually trigger the CI builds.

*   [TensorFlow Lite 8-bit quantization specification](https://www.tensorflow.org/lite/performance/quantization_spec)

[#45387]: https://github.com/tensorflow/tensorflow/issues/45387


## General Guidelines

TODO: Introduction

Be sure to observe the [micro Contributing Guidelines][].

[micro Contributing Guidelines]: https://github.com/tensorflow/tensorflow/blob/master/tensorflow/lite/micro/CONTRIBUTING.md

### Check each commit for formatting, lint, and unit-test passage

Check each commit against the [pre-submit checklist][] in the micro
Contributing Guidelines. Specifically, make sure your code:

1.  Is formatted with clang-format.
1.  Passes a lint check.
1.  Passes all unit tests.

CI runs these checks on all PRs, and will hold up your PR if any of these checks fail.

[pre-submit checklist]: https://github.com/tensorflow/tensorflow/blob/master/tensorflow/lite/micro/CONTRIBUTING.md#before-submitting-your-pr

### Add yourself as an asignee to PRs

Feel free to add yourself as an additional assignee to PRs which you submit.
Other assignees may be set by the project's various bots.

### Maintain a 1:1 correspondence between micro and lite versions of unit tests

To the extent possible, maintain a 1:1 correspondence between micro and lite
versions of unit tests. Avoid cleanup of merely stylistic issues, e.g., by
replacing the hardcoded literal `3.40282e+038` with
`std::numeric_limits<float>::max()`. Any changes between the micro and lite
versions of a test put a burden on future maintainers to figure out whether the
differences are actually significant or just stylistic.

### Sometimes CI checks on PRs are flakey and fail

Sometimes CI checks on PRs don't fail because of the PRs contents, but because
of some problem with the test infrastructure. Marking issues with the label
`kokoro:force-run` causes the checks to be rerun.
