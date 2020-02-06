
struct PS_INPUT
{
    ${PSINPUT}
};

struct PS_OUTPUT
{
    ${PSOUTPUT}
};

${PSPARAMETERS}

PS_OUTPUT main(PS_INPUT input)
{
    PS_OUTPUT output;

    ${PSBODY}

    return output;
}