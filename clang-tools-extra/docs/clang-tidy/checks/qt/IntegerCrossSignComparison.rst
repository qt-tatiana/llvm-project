.. title:: clang-tidy - qt-integer-sign-comparison

qt-integer-sign-comparison
=============================

The check detects comparison between signed and unsigned integer values and suggests a fix-it.

Examples of fixes created by the check:

.. code-block:: c++

    uint func(int bla)
    {
        uint result;
        if (result == bla)
            return 0;

        return 1;
    }

becomes

.. code-block:: c++

uint func(int bla)
{
    uint result;
    if (std::cmp_equal(result, bla))
        return 0;

    return 1;
}
