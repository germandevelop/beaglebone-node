/************************************************************
 *   Author : German Mundinger
 *   Date   : 2024
 ************************************************************/

#include <gmock/gmock.h>

#include "NodeB01.hpp"


class NodeB01TestFixture : public testing::Test
{
    protected:
        NodeB01 node { NodeB01::Config { .isWarningEnabled = true } };
};

TEST_F(NodeB01TestFixture, Init)
{
    // Arrange: create and set up a system under test
    NodeB01::State expectedState;
    expectedState.isMessageToSend   = false;
    expectedState.statusLedColor    = STATUS_LED_COLOR::GREEN;
    expectedState.isLightON         = false;
    expectedState.isDisplayON       = false;
    expectedState.isWarningAudio    = false;
    expectedState.isIntrusionAudio  = false;
    expectedState.isAlarmAudio      = false;

    // Act: poke the system under test
    NodeB01::State resultState = node.getState(NodeB01::DISPLAY_DURATION_S * 1000U);

    // Assert: make unit test pass or fail
    EXPECT_EQ(resultState.isMessageToSend,  expectedState.isMessageToSend);
    EXPECT_EQ(resultState.statusLedColor,   expectedState.statusLedColor);
    EXPECT_EQ(resultState.isLightON,        expectedState.isLightON);
    EXPECT_EQ(resultState.isDisplayON,      expectedState.isDisplayON);
    EXPECT_EQ(resultState.isWarningAudio,   expectedState.isWarningAudio);
    EXPECT_EQ(resultState.isIntrusionAudio, expectedState.isIntrusionAudio);
    EXPECT_EQ(resultState.isAlarmAudio,     expectedState.isAlarmAudio);
}


TEST_F(NodeB01TestFixture, SendPeriodicMessages)
{
    // Arrange: create and set up a system under test
    NodeB01::State expectedState;
    expectedState.isMessageToSend = true;

    // Act: poke the system under test
    NodeB01::State resultState = node.getState((NodeB01::MESSAGE_PERIOD_MIN * 60U * 1000U) + 1U);

    // Assert: make unit test pass or fail
    EXPECT_EQ(resultState.isMessageToSend, expectedState.isMessageToSend);
}


class NodeB01ParamLuminosity : public NodeB01TestFixture, public testing::WithParamInterface
    <std::tuple<
        NodeB01::Luminosity,
        bool
    >>
{};

TEST_P(NodeB01ParamLuminosity, ProcessLuminosity)
{
    // Arrange: create and set up a system under test
    NodeB01::Luminosity luminosity = std::get<0>(GetParam());

    bool expectedIsDark = std::get<1>(GetParam());

    // Act: poke the system under test
    node.processLuminosity(luminosity);
    const bool resultIsDark = node.getDarkness();

    // Assert: make unit test pass or fail
    EXPECT_EQ(resultIsDark, expectedIsDark);
}

INSTANTIATE_TEST_SUITE_P(NodeB01TestFixture, NodeB01ParamLuminosity,
    testing::Values
    (
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX),       .isValid = false },   false),
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX - 1.0F),.isValid = false },   false),
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX),       .isValid = true },    false),
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX - 1.0F),.isValid = true },    true)
    )
);


class NodeB01ParamRemoteControlMode : public NodeB01TestFixture, public testing::WithParamInterface
    <std::tuple<
        NodeB01::Luminosity,
        REMOTE_CONTROL_BUTTON,
        NodeB01::State
    >>
{};

TEST_P(NodeB01ParamRemoteControlMode, ProcessRemoteButtonMode)
{
    // Arrange: create and set up a system under test
    NodeB01::Luminosity luminosity = std::get<0>(GetParam());
    node.processLuminosity(luminosity);

    REMOTE_CONTROL_BUTTON button = std::get<1>(GetParam());

    NodeB01::State expectedState = std::get<2>(GetParam());

    // Act: poke the system under test
    node.processRemoteButton(button, (NodeB01::DISPLAY_DURATION_S * 2U * 1000U));
    NodeB01::State resultState = node.getState(((NodeB01::DISPLAY_DURATION_S * 2U * 1000U) + 1U));

    // Assert: make unit test pass or fail
    EXPECT_EQ(resultState.isMessageToSend,  expectedState.isMessageToSend);
    EXPECT_EQ(resultState.statusLedColor,   expectedState.statusLedColor);
    EXPECT_EQ(resultState.isLightON,        expectedState.isLightON);
    EXPECT_EQ(resultState.isDisplayON,      expectedState.isDisplayON);
    EXPECT_EQ(resultState.isWarningAudio,   expectedState.isWarningAudio);
    EXPECT_EQ(resultState.isIntrusionAudio, expectedState.isIntrusionAudio);
    EXPECT_EQ(resultState.isAlarmAudio,     expectedState.isAlarmAudio);
}

INSTANTIATE_TEST_SUITE_P(NodeB01TestFixture, NodeB01ParamRemoteControlMode,
    testing::Values
    (
        // SILENCE mode
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX), .isValid = true },
                        REMOTE_CONTROL_BUTTON::ONE,
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::GREEN, .isLightON = false,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = false, .isMessageToSend = true }),
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX - 1.0F), .isValid = true },
                        REMOTE_CONTROL_BUTTON::ONE,
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::GREEN, .isLightON = false,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = false, .isMessageToSend = true }),
        // GUARD mode
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX), .isValid = true },
                        REMOTE_CONTROL_BUTTON::TWO,
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::RED, .isLightON = false,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = false, .isMessageToSend = true }),
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX - 1.0F), .isValid = true },
                        REMOTE_CONTROL_BUTTON::TWO,
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::RED, .isLightON = false,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = false, .isMessageToSend = true }),
        // ALARM mode
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX), .isValid = true },
                        REMOTE_CONTROL_BUTTON::GRID,
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::RED, .isLightON = false,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = true, .isMessageToSend = true }),
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX - 1.0F), .isValid = true },
                        REMOTE_CONTROL_BUTTON::GRID,
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::RED, .isLightON = true,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = true, .isMessageToSend = true })
    )
);


class NodeB01ParamRemoteControl : public NodeB01TestFixture, public testing::WithParamInterface
    <std::tuple<
        NodeB01::Luminosity,
        REMOTE_CONTROL_BUTTON,
        REMOTE_CONTROL_BUTTON,
        NodeB01::State
    >>
{};

TEST_P(NodeB01ParamRemoteControl, ProcessRemoteButton)
{
    // Arrange: create and set up a system under test
    NodeB01::Luminosity luminosity = std::get<0>(GetParam());
    node.processLuminosity(luminosity);

    REMOTE_CONTROL_BUTTON modeButton = std::get<1>(GetParam());
    node.processRemoteButton(modeButton, (NodeB01::DISPLAY_DURATION_S * 2U * 1000U));
    node.extractMessages();

    REMOTE_CONTROL_BUTTON button = std::get<2>(GetParam());

    NodeB01::State expectedState = std::get<3>(GetParam());

    // Act: poke the system under test
    node.processRemoteButton(button, ((NodeB01::DISPLAY_DURATION_S * 2U * 1000U) + 1U));
    NodeB01::State resultState = node.getState(((NodeB01::DISPLAY_DURATION_S * 2U * 1000U) + 2U));

    // Assert: make unit test pass or fail
    EXPECT_EQ(resultState.isMessageToSend,  expectedState.isMessageToSend);
    EXPECT_EQ(resultState.statusLedColor,   expectedState.statusLedColor);
    EXPECT_EQ(resultState.isLightON,        expectedState.isLightON);
    EXPECT_EQ(resultState.isDisplayON,      expectedState.isDisplayON);
    EXPECT_EQ(resultState.isWarningAudio,   expectedState.isWarningAudio);
    EXPECT_EQ(resultState.isIntrusionAudio, expectedState.isIntrusionAudio);
    EXPECT_EQ(resultState.isAlarmAudio,     expectedState.isAlarmAudio);
}

INSTANTIATE_TEST_SUITE_P(NodeB01TestFixture, NodeB01ParamRemoteControl,
    testing::Values
    (
        // SILENCE mode + disable intrusion
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX), .isValid = true },
                        REMOTE_CONTROL_BUTTON::ONE, REMOTE_CONTROL_BUTTON::THREE,
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::GREEN, .isLightON = false,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = false, .isMessageToSend = true }),
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX - 1.0F), .isValid = true },
                        REMOTE_CONTROL_BUTTON::ONE, REMOTE_CONTROL_BUTTON::THREE,
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::GREEN, .isLightON = false,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = false, .isMessageToSend = true }),
        // GUARD mode + disable intrusion
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX), .isValid = true },
                        REMOTE_CONTROL_BUTTON::TWO, REMOTE_CONTROL_BUTTON::THREE,
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::RED, .isLightON = false,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = false, .isMessageToSend = true }),
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX - 1.0F), .isValid = true },
                        REMOTE_CONTROL_BUTTON::TWO, REMOTE_CONTROL_BUTTON::THREE,
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::RED, .isLightON = false,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = false, .isMessageToSend = true }),
        // ALARM mode + disable intrusion
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX), .isValid = true },
                        REMOTE_CONTROL_BUTTON::GRID, REMOTE_CONTROL_BUTTON::THREE,
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::RED, .isLightON = false,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = true, .isMessageToSend = true }),
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX - 1.0F), .isValid = true },
                        REMOTE_CONTROL_BUTTON::GRID, REMOTE_CONTROL_BUTTON::THREE,
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::RED, .isLightON = true,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = true, .isMessageToSend = true }),

        // SILENCE mode + enable light
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX), .isValid = true },
                        REMOTE_CONTROL_BUTTON::ONE, REMOTE_CONTROL_BUTTON::FOUR,
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::GREEN, .isLightON = false,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = false, .isMessageToSend = true }),
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX - 1.0F), .isValid = true },
                        REMOTE_CONTROL_BUTTON::ONE, REMOTE_CONTROL_BUTTON::FOUR,
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::GREEN, .isLightON = true,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = false, .isMessageToSend = true }),
        // GUARD mode  + enable light
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX), .isValid = true },
                        REMOTE_CONTROL_BUTTON::TWO, REMOTE_CONTROL_BUTTON::FOUR,
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::RED, .isLightON = false,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = false, .isMessageToSend = true }),
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX - 1.0F), .isValid = true },
                        REMOTE_CONTROL_BUTTON::TWO, REMOTE_CONTROL_BUTTON::FOUR,
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::RED, .isLightON = false,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = false, .isMessageToSend = true }),
        // ALARM mode + enable light
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX), .isValid = true },
                        REMOTE_CONTROL_BUTTON::GRID, REMOTE_CONTROL_BUTTON::FOUR,
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::RED, .isLightON = false,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = true, .isMessageToSend = false }),
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX - 1.0F), .isValid = true },
                        REMOTE_CONTROL_BUTTON::GRID, REMOTE_CONTROL_BUTTON::FOUR,
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::RED, .isLightON = true,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = true, .isMessageToSend = false }),

        // SILENCE mode + enable display
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX), .isValid = true },
                        REMOTE_CONTROL_BUTTON::ONE, REMOTE_CONTROL_BUTTON::FIVE,
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::GREEN, .isLightON = false,
                                        .isDisplayON = true, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = false, .isMessageToSend = false }),
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX - 1.0F), .isValid = true },
                        REMOTE_CONTROL_BUTTON::ONE, REMOTE_CONTROL_BUTTON::FIVE,
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::GREEN, .isLightON = false,
                                        .isDisplayON = true, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = false, .isMessageToSend = false }),
        // GUARD mode  + enable display
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX), .isValid = true },
                        REMOTE_CONTROL_BUTTON::TWO, REMOTE_CONTROL_BUTTON::FIVE,
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::RED, .isLightON = false,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = false, .isMessageToSend = false }),
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX - 1.0F), .isValid = true },
                        REMOTE_CONTROL_BUTTON::TWO, REMOTE_CONTROL_BUTTON::FIVE,
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::RED, .isLightON = false,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = false, .isMessageToSend = false }),
        // ALARM mode + enable display
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX), .isValid = true },
                        REMOTE_CONTROL_BUTTON::GRID, REMOTE_CONTROL_BUTTON::FIVE,
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::RED, .isLightON = false,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = true, .isMessageToSend = false }),
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX - 1.0F), .isValid = true },
                        REMOTE_CONTROL_BUTTON::GRID, REMOTE_CONTROL_BUTTON::FIVE,
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::RED, .isLightON = true,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = true, .isMessageToSend = false })
    )
);

TEST_F(NodeB01TestFixture, ProcessRemoteButtonWarningOff)
{
    // Arrange: create and set up a system under test
    REMOTE_CONTROL_BUTTON button = REMOTE_CONTROL_BUTTON::ZERO;

    NodeB01::Config expectedConfig;
    expectedConfig.isWarningEnabled = false;

    // Act: poke the system under test
    node.processRemoteButton(button, ((NodeB01::DISPLAY_DURATION_S * 2U * 1000U) + 1U));
    NodeB01::Config resultConfig = node.getConfig();

    // Assert: make unit test pass or fail
    EXPECT_EQ(resultConfig.isWarningEnabled, expectedConfig.isWarningEnabled);
}

TEST_F(NodeB01TestFixture, ProcessRemoteButtonWarningOn)
{
    // Arrange: create and set up a system under test
    REMOTE_CONTROL_BUTTON button = REMOTE_CONTROL_BUTTON::ZERO;

    node.processRemoteButton(button, (NodeB01::DISPLAY_DURATION_S * 2U * 1000U));
    node.extractMessages();

    NodeB01::Config expectedConfig;
    expectedConfig.isWarningEnabled = true;

    // Act: poke the system under test
    node.processRemoteButton(button, ((NodeB01::DISPLAY_DURATION_S * 2U * 1000U) + 1U));
    NodeB01::Config resultConfig = node.getConfig();

    // Assert: make unit test pass or fail
    EXPECT_EQ(resultConfig.isWarningEnabled, expectedConfig.isWarningEnabled);
}


class NodeB01ParamDoorPir : public NodeB01TestFixture, public testing::WithParamInterface
    <std::tuple<
        NodeB01::Luminosity,
        REMOTE_CONTROL_BUTTON,
        NodeB01::State
    >>
{};

TEST_P(NodeB01ParamDoorPir, ProcessDoorPir)
{
    // Arrange: create and set up a system under test
    NodeB01::Luminosity luminosity = std::get<0>(GetParam());
    node.processLuminosity(luminosity);

    REMOTE_CONTROL_BUTTON modeButton = std::get<1>(GetParam());
    node.processRemoteButton(modeButton, (NodeB01::DISPLAY_DURATION_S * 2U * 1000U));
    node.extractMessages();

    NodeB01::State expectedState = std::get<2>(GetParam());

    // Act: poke the system under test
    node.processDoorMovement(((NodeB01::DISPLAY_DURATION_S * 2U * 1000U) + 1U));
    NodeB01::State resultState = node.getState(((NodeB01::DISPLAY_DURATION_S * 2U * 1000U) + 2U));

    // Assert: make unit test pass or fail
    EXPECT_EQ(resultState.isMessageToSend,  expectedState.isMessageToSend);
    EXPECT_EQ(resultState.statusLedColor,   expectedState.statusLedColor);
    EXPECT_EQ(resultState.isLightON,        expectedState.isLightON);
    EXPECT_EQ(resultState.isDisplayON,      expectedState.isDisplayON);
    EXPECT_EQ(resultState.isWarningAudio,   expectedState.isWarningAudio);
    EXPECT_EQ(resultState.isIntrusionAudio, expectedState.isIntrusionAudio);
    EXPECT_EQ(resultState.isAlarmAudio,     expectedState.isAlarmAudio);
}

INSTANTIATE_TEST_SUITE_P(NodeB01TestFixture, NodeB01ParamDoorPir,
    testing::Values
    (
        // SILENCE mode + door pir
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX), .isValid = true },
                        REMOTE_CONTROL_BUTTON::ONE,
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::GREEN, .isLightON = false,
                                        .isDisplayON = true, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = false, .isMessageToSend = false }),
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX - 1.0F), .isValid = true },
                        REMOTE_CONTROL_BUTTON::ONE,
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::GREEN, .isLightON = true,
                                        .isDisplayON = true, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = false, .isMessageToSend = true }),
        // GUARD mode + door pir
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX), .isValid = true },
                        REMOTE_CONTROL_BUTTON::TWO,
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::RED, .isLightON = false,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = false, .isMessageToSend = false }),
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX - 1.0F), .isValid = true },
                        REMOTE_CONTROL_BUTTON::TWO,
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::RED, .isLightON = false,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = false, .isMessageToSend = false }),
        // ALARM mode + door pir
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX), .isValid = true },
                        REMOTE_CONTROL_BUTTON::GRID,
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::RED, .isLightON = false,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = true, .isMessageToSend = false }),
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX - 1.0F), .isValid = true },
                        REMOTE_CONTROL_BUTTON::GRID,
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::RED, .isLightON = true,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = true, .isMessageToSend = false })
    )
);


class NodeB01ParamRoomPir : public NodeB01TestFixture, public testing::WithParamInterface
    <std::tuple<
        NodeB01::Luminosity,
        REMOTE_CONTROL_BUTTON,
        NodeB01::State
    >>
{};

TEST_P(NodeB01ParamRoomPir, ProcessRoomPir)
{
    // Arrange: create and set up a system under test
    NodeB01::Luminosity luminosity = std::get<0>(GetParam());
    node.processLuminosity(luminosity);

    REMOTE_CONTROL_BUTTON modeButton = std::get<1>(GetParam());
    node.processRemoteButton(modeButton, (NodeB01::DISPLAY_DURATION_S * 2U * 1000U));
    node.extractMessages();

    NodeB01::State expectedState = std::get<2>(GetParam());

    // Act: poke the system under test
    node.processRoomMovement(((NodeB01::DISPLAY_DURATION_S * 2U * 1000U) + 1U));
    NodeB01::State resultState = node.getState(((NodeB01::DISPLAY_DURATION_S * 2U * 1000U) + 2U));

    // Assert: make unit test pass or fail
    EXPECT_EQ(resultState.isMessageToSend,  expectedState.isMessageToSend);
    EXPECT_EQ(resultState.statusLedColor,   expectedState.statusLedColor);
    EXPECT_EQ(resultState.isLightON,        expectedState.isLightON);
    EXPECT_EQ(resultState.isDisplayON,      expectedState.isDisplayON);
    EXPECT_EQ(resultState.isWarningAudio,   expectedState.isWarningAudio);
    EXPECT_EQ(resultState.isIntrusionAudio, expectedState.isIntrusionAudio);
    EXPECT_EQ(resultState.isAlarmAudio,     expectedState.isAlarmAudio);
}

INSTANTIATE_TEST_SUITE_P(NodeB01TestFixture, NodeB01ParamRoomPir,
    testing::Values
    (
        // SILENCE mode + room pir
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX), .isValid = true },
                        REMOTE_CONTROL_BUTTON::ONE,
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::GREEN, .isLightON = false,
                                        .isDisplayON = true, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = false, .isMessageToSend = false }),
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX - 1.0F), .isValid = true },
                        REMOTE_CONTROL_BUTTON::ONE,
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::GREEN, .isLightON = false,
                                        .isDisplayON = true, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = false, .isMessageToSend = false }),
        // GUARD mode + room pir
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX), .isValid = true },
                        REMOTE_CONTROL_BUTTON::TWO,
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::RED, .isLightON = false,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = false, .isMessageToSend = false }),
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX - 1.0F), .isValid = true },
                        REMOTE_CONTROL_BUTTON::TWO,
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::RED, .isLightON = false,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = false, .isMessageToSend = false }),
        // ALARM mode + room pir
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX), .isValid = true },
                        REMOTE_CONTROL_BUTTON::GRID,
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::RED, .isLightON = false,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = true, .isMessageToSend = false }),
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX - 1.0F), .isValid = true },
                        REMOTE_CONTROL_BUTTON::GRID,
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::RED, .isLightON = true,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = true, .isMessageToSend = false })
    )
);


class NodeB01ParamSmoke : public NodeB01TestFixture, public testing::WithParamInterface
    <std::tuple<
        NodeB01::Luminosity,
        REMOTE_CONTROL_BUTTON,
        PeriodicSmokeSensorData,
        NodeB01::State
    >>
{};

TEST_P(NodeB01ParamSmoke, ProcessSmoke)
{
    // Arrange: create and set up a system under test
    NodeB01::Luminosity luminosity = std::get<0>(GetParam());
    node.processLuminosity(luminosity);

    REMOTE_CONTROL_BUTTON modeButton = std::get<1>(GetParam());
    node.processRemoteButton(modeButton, (NodeB01::DISPLAY_DURATION_S * 2U * 1000U));
    node.extractMessages();

    PeriodicSmokeSensorData data = std::get<2>(GetParam());

    NodeB01::State expectedState = std::get<3>(GetParam());

    // Act: poke the system under test
    node.processSmoke(data);
    NodeB01::State resultState = node.getState(((NodeB01::DISPLAY_DURATION_S * 2U * 1000U) + 1U));

    // Assert: make unit test pass or fail
    EXPECT_EQ(resultState.isMessageToSend,  expectedState.isMessageToSend);
    EXPECT_EQ(resultState.statusLedColor,   expectedState.statusLedColor);
    EXPECT_EQ(resultState.isLightON,        expectedState.isLightON);
    EXPECT_EQ(resultState.isDisplayON,      expectedState.isDisplayON);
    EXPECT_EQ(resultState.isWarningAudio,   expectedState.isWarningAudio);
    EXPECT_EQ(resultState.isIntrusionAudio, expectedState.isIntrusionAudio);
    EXPECT_EQ(resultState.isAlarmAudio,     expectedState.isAlarmAudio);
}

INSTANTIATE_TEST_SUITE_P(NodeB01TestFixture, NodeB01ParamSmoke,
    testing::Values
    (
        // SILENCE mode
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX), .isValid = true },
                        REMOTE_CONTROL_BUTTON::ONE,
                        PeriodicSmokeSensorData { .adcValue = (NodeB01::SMOKE_THRESHOLD_ADC + 1U), .isValid = true },
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::RED, .isLightON = false,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = true, .isMessageToSend = true }),
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX - 1.0F), .isValid = true },
                        REMOTE_CONTROL_BUTTON::ONE,
                        PeriodicSmokeSensorData { .adcValue = (NodeB01::SMOKE_THRESHOLD_ADC + 1U), .isValid = true },
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::RED, .isLightON = true,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = true, .isMessageToSend = true }),
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX - 1.0F), .isValid = true },
                        REMOTE_CONTROL_BUTTON::ONE,
                        PeriodicSmokeSensorData { .adcValue = (NodeB01::SMOKE_THRESHOLD_ADC), .isValid = true },
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::GREEN, .isLightON = false,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = false, .isMessageToSend = false }),
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX - 1.0F), .isValid = true },
                        REMOTE_CONTROL_BUTTON::ONE,
                        PeriodicSmokeSensorData { .isValid = false },
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::GREEN, .isLightON = false,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = false, .isMessageToSend = false }),

        // GUARD mode
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX), .isValid = true },
                        REMOTE_CONTROL_BUTTON::TWO,
                        PeriodicSmokeSensorData { .adcValue = (NodeB01::SMOKE_THRESHOLD_ADC + 1U), .isValid = true },
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::RED, .isLightON = false,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = true, .isMessageToSend = true }),
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX - 1.0F), .isValid = true },
                        REMOTE_CONTROL_BUTTON::TWO,
                        PeriodicSmokeSensorData { .adcValue = (NodeB01::SMOKE_THRESHOLD_ADC + 1U), .isValid = true },
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::RED, .isLightON = true,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = true, .isMessageToSend = true }),
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX - 1.0F), .isValid = true },
                        REMOTE_CONTROL_BUTTON::TWO,
                        PeriodicSmokeSensorData { .adcValue = (NodeB01::SMOKE_THRESHOLD_ADC), .isValid = true },
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::RED, .isLightON = false,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = false, .isMessageToSend = false }),
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX - 1.0F), .isValid = true },
                        REMOTE_CONTROL_BUTTON::TWO,
                        PeriodicSmokeSensorData { .isValid = false },
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::RED, .isLightON = false,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = false, .isMessageToSend = false }),

        // ALARM mode
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX), .isValid = true },
                        REMOTE_CONTROL_BUTTON::GRID,
                        PeriodicSmokeSensorData { .adcValue = (NodeB01::SMOKE_THRESHOLD_ADC + 1U), .isValid = true },
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::RED, .isLightON = false,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = true, .isMessageToSend = true }),
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX - 1.0F), .isValid = true },
                        REMOTE_CONTROL_BUTTON::GRID,
                        PeriodicSmokeSensorData { .adcValue = (NodeB01::SMOKE_THRESHOLD_ADC + 1U), .isValid = true },
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::RED, .isLightON = true,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = true, .isMessageToSend = true }),
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX - 1.0F), .isValid = true },
                        REMOTE_CONTROL_BUTTON::GRID,
                        PeriodicSmokeSensorData { .adcValue = (NodeB01::SMOKE_THRESHOLD_ADC), .isValid = true },
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::GREEN, .isLightON = false,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = false, .isMessageToSend = true }),
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX - 1.0F), .isValid = true },
                        REMOTE_CONTROL_BUTTON::GRID,
                        PeriodicSmokeSensorData { .isValid = false },
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::GREEN, .isLightON = false,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = false, .isMessageToSend = true })
    )
);


class NodeB01ParamMsgCmd : public NodeB01TestFixture, public testing::WithParamInterface
    <std::tuple<
        NodeB01::Luminosity,
        REMOTE_CONTROL_BUTTON,
        NodeMsg,
        NodeB01::State
    >>
{};

TEST_P(NodeB01ParamMsgCmd, ProcessMsgCmd)
{
    // Arrange: create and set up a system under test
    NodeB01::Luminosity luminosity = std::get<0>(GetParam());
    node.processLuminosity(luminosity);

    REMOTE_CONTROL_BUTTON modeButton = std::get<1>(GetParam());
    node.processRemoteButton(modeButton, (NodeB01::DISPLAY_DURATION_S * 2U * 1000U));
    node.extractMessages();

    NodeMsg msg = std::get<2>(GetParam());

    NodeB01::State expectedState = std::get<3>(GetParam());

    // Act: poke the system under test
    node.processMessage(msg, ((NodeB01::DISPLAY_DURATION_S * 2U * 1000U) + 1U));
    NodeB01::State resultState = node.getState(((NodeB01::DISPLAY_DURATION_S * 2U * 1000U) + 2U));

    // Assert: make unit test pass or fail
    EXPECT_EQ(resultState.isMessageToSend,  expectedState.isMessageToSend);
    EXPECT_EQ(resultState.statusLedColor,   expectedState.statusLedColor);
    EXPECT_EQ(resultState.isLightON,        expectedState.isLightON);
    EXPECT_EQ(resultState.isDisplayON,      expectedState.isDisplayON);
    EXPECT_EQ(resultState.isWarningAudio,   expectedState.isWarningAudio);
    EXPECT_EQ(resultState.isIntrusionAudio, expectedState.isIntrusionAudio);
    EXPECT_EQ(resultState.isAlarmAudio,     expectedState.isAlarmAudio);
}

INSTANTIATE_TEST_SUITE_P(NodeB01TestFixture, NodeB01ParamMsgCmd,
    testing::Values
    (
        // SILENCE mode
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX), .isValid = true },
                        REMOTE_CONTROL_BUTTON::ONE,
                        NodeMsg { .header { .destArray { NODE_B01 } }, .cmdID = SET_INTRUSION,
                                    .dataArray { { "value_id", static_cast<int>(INTRUSION_ON) } } },
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::GREEN, .isLightON = false,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = false, .isMessageToSend = false }),
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX - 1.0F), .isValid = true },
                        REMOTE_CONTROL_BUTTON::ONE,
                        NodeMsg { .header { .destArray { NODE_B01 } }, .cmdID = SET_INTRUSION,
                                    .dataArray { { "value_id", static_cast<int>(INTRUSION_ON) } } },
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::GREEN, .isLightON = false,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = false, .isMessageToSend = false }),
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX), .isValid = true },
                        REMOTE_CONTROL_BUTTON::ONE,
                        NodeMsg { .header { .destArray { NODE_B01 } }, .cmdID = SET_LIGHT,
                                    .dataArray { { "value_id", static_cast<int>(LIGHT_ON) } } },
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::GREEN, .isLightON = false,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = false, .isMessageToSend = false }),
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX - 1.0F), .isValid = true },
                        REMOTE_CONTROL_BUTTON::ONE,
                        NodeMsg { .header { .destArray { NODE_B01 } }, .cmdID = SET_LIGHT,
                                    .dataArray { { "value_id", static_cast<int>(LIGHT_ON) } } },
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::GREEN, .isLightON = true,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = false, .isMessageToSend = false }),

        // GUARD mode
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX), .isValid = true },
                        REMOTE_CONTROL_BUTTON::TWO,
                        NodeMsg { .header { .destArray { NODE_B01 } }, .cmdID = SET_INTRUSION,
                                    .dataArray { { "value_id", static_cast<int>(INTRUSION_ON) } } },
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::RED, .isLightON = false,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = true,
                                        .isAlarmAudio = false, .isMessageToSend = false }),
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX - 1.0F), .isValid = true },
                        REMOTE_CONTROL_BUTTON::TWO,
                        NodeMsg { .header { .destArray { NODE_B01 } }, .cmdID = SET_INTRUSION,
                                    .dataArray { { "value_id", static_cast<int>(INTRUSION_ON) } } },
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::RED, .isLightON = false,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = true,
                                        .isAlarmAudio = false, .isMessageToSend = false }),
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX), .isValid = true },
                        REMOTE_CONTROL_BUTTON::TWO,
                        NodeMsg { .header { .destArray { NODE_B01 } }, .cmdID = SET_LIGHT,
                                    .dataArray { { "value_id", static_cast<int>(LIGHT_ON) } } },
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::RED, .isLightON = false,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = false, .isMessageToSend = false }),
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX - 1.0F), .isValid = true },
                        REMOTE_CONTROL_BUTTON::TWO,
                        NodeMsg { .header { .destArray { NODE_B01 } }, .cmdID = SET_LIGHT,
                                    .dataArray { { "value_id", static_cast<int>(LIGHT_ON) } } },
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::RED, .isLightON = false,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = false, .isMessageToSend = false }),

        // ALARM mode
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX), .isValid = true },
                        REMOTE_CONTROL_BUTTON::GRID,
                        NodeMsg { .header { .destArray { NODE_B01 } }, .cmdID = SET_INTRUSION,
                                    .dataArray { { "value_id", static_cast<int>(INTRUSION_ON) } } },
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::RED, .isLightON = false,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = true, .isMessageToSend = false }),
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX - 1.0F), .isValid = true },
                        REMOTE_CONTROL_BUTTON::GRID,
                        NodeMsg { .header { .destArray { NODE_B01 } }, .cmdID = SET_INTRUSION,
                                    .dataArray { { "value_id", static_cast<int>(INTRUSION_ON) } } },
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::RED, .isLightON = true,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = true, .isMessageToSend = false }),
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX), .isValid = true },
                        REMOTE_CONTROL_BUTTON::GRID,
                        NodeMsg { .header { .destArray { NODE_B01 } }, .cmdID = SET_LIGHT,
                                    .dataArray { { "value_id", static_cast<int>(LIGHT_ON) } } },
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::RED, .isLightON = false,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = true, .isMessageToSend = false }),
        std::make_tuple(NodeB01::Luminosity { .lux = (NodeB01::DARKNESS_LEVEL_LUX - 1.0F), .isValid = true },
                        REMOTE_CONTROL_BUTTON::GRID,
                        NodeMsg { .header { .destArray { NODE_B01 } }, .cmdID = SET_LIGHT,
                                    .dataArray { { "value_id", static_cast<int>(LIGHT_ON) } } },
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::RED, .isLightON = true,
                                        .isDisplayON = false, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = true, .isMessageToSend = false })
    )
);


class NodeB01ParamMsgT01 : public NodeB01TestFixture, public testing::WithParamInterface
    <std::tuple<
        NodeB01::Config,
        NodeMsg,
        NodeMsg,
        NodeB01::State
    >>
{};

TEST_P(NodeB01ParamMsgT01, ProcessMsgT01)
{
    // Arrange: create and set up a system under test
    NodeB01::Config config = std::get<0>(GetParam());
    node.setConfig(config);

    NodeMsg humidityMsg = std::get<1>(GetParam());
    NodeMsg doorMsg     = std::get<2>(GetParam());

    NodeB01::State expectedState = std::get<3>(GetParam());

    // Act: poke the system under test
    node.processMessage(humidityMsg, (NodeB01::DISPLAY_DURATION_S * 2U * 1000U));
    node.processMessage(doorMsg, ((NodeB01::DISPLAY_DURATION_S * 2U * 1000U) + 1U));
    node.processDoorMovement(((NodeB01::DISPLAY_DURATION_S * 2U * 1000U) + 2U));
    NodeB01::State resultState = node.getState(((NodeB01::DISPLAY_DURATION_S * 2U * 1000U) + 3U));

    // Assert: make unit test pass or fail
    EXPECT_EQ(resultState.isMessageToSend,  expectedState.isMessageToSend);
    EXPECT_EQ(resultState.statusLedColor,   expectedState.statusLedColor);
    EXPECT_EQ(resultState.isLightON,        expectedState.isLightON);
    EXPECT_EQ(resultState.isDisplayON,      expectedState.isDisplayON);
    EXPECT_EQ(resultState.isWarningAudio,   expectedState.isWarningAudio);
    EXPECT_EQ(resultState.isIntrusionAudio, expectedState.isIntrusionAudio);
    EXPECT_EQ(resultState.isAlarmAudio,     expectedState.isAlarmAudio);
}

INSTANTIATE_TEST_SUITE_P(NodeB01TestFixture, NodeB01ParamMsgT01,
    testing::Values
    (
        std::make_tuple(NodeB01::Config { .isWarningEnabled = false },
                        NodeMsg { .header { .source = NODE_T01, .destArray { NODE_B01 } },
                                .cmdID = UPDATE_TEMPERATURE,
                                .dataArray { { "temp_c", static_cast<float>(0.0F) },
                                             { "pres_hpa", static_cast<int>(1000) },
                                             { "hum_pct", static_cast<int>(100) } } },
                        NodeMsg { .header { .source = NODE_T01, .destArray { NODE_B01 } },
                                .cmdID = UPDATE_DOOR_STATE,
                                .dataArray { { "door_state", static_cast<int>(false) } } },
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::GREEN, .isLightON = false,
                                        .isDisplayON = true, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = false, .isMessageToSend = false }),

        std::make_tuple(NodeB01::Config { .isWarningEnabled = true },
                        NodeMsg { .header { .source = NODE_T01, .destArray { NODE_B01 } },
                                .cmdID = UPDATE_TEMPERATURE,
                                .dataArray { { "temp_c", static_cast<float>(NodeB01::T01_LOW_TEMPERATURE_C - 1.0F) },
                                             { "pres_hpa", static_cast<int>(1000) },
                                             { "hum_pct", static_cast<int>(100) } } },
                        NodeMsg { .header { .source = NODE_T01, .destArray { NODE_B01 } },
                                .cmdID = UPDATE_DOOR_STATE,
                                .dataArray { { "door_state", static_cast<int>(true) } } },
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::GREEN, .isLightON = false,
                                        .isDisplayON = true, .isWarningAudio = true, .isIntrusionAudio = false,
                                        .isAlarmAudio = false, .isMessageToSend = false }),

        std::make_tuple(NodeB01::Config { .isWarningEnabled = true },
                        NodeMsg { .header { .source = NODE_T01, .destArray { NODE_B01 } },
                                .cmdID = UPDATE_TEMPERATURE,
                                .dataArray { { "temp_c", static_cast<float>(NodeB01::T01_LOW_TEMPERATURE_C + 1.0F) },
                                             { "pres_hpa", static_cast<int>(1000) },
                                             { "hum_pct", static_cast<int>(100) } } },
                        NodeMsg { .header { .source = NODE_T01, .destArray { NODE_B01 } },
                                .cmdID = UPDATE_DOOR_STATE,
                                .dataArray { { "door_state", static_cast<int>(true) } } },
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::GREEN, .isLightON = false,
                                        .isDisplayON = true, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = false, .isMessageToSend = false }),

        std::make_tuple(NodeB01::Config { .isWarningEnabled = true },
                        NodeMsg { .header { .source = NODE_T01, .destArray { NODE_B01 } },
                                .cmdID = UPDATE_TEMPERATURE,
                                .dataArray { { "temp_c", static_cast<float>(NodeB01::T01_HIGH_TEMPERATURE_C - 1.0F) },
                                             { "pres_hpa", static_cast<int>(1000) },
                                             { "hum_pct", static_cast<int>(100) } } },
                        NodeMsg { .header { .source = NODE_T01, .destArray { NODE_B01 } },
                                .cmdID = UPDATE_DOOR_STATE,
                                .dataArray { { "door_state", static_cast<int>(false) } } },
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::GREEN, .isLightON = false,
                                        .isDisplayON = true, .isWarningAudio = false, .isIntrusionAudio = false,
                                        .isAlarmAudio = false, .isMessageToSend = false }),

        std::make_tuple(NodeB01::Config { .isWarningEnabled = true },
                        NodeMsg { .header { .source = NODE_T01, .destArray { NODE_B01 } },
                                .cmdID = UPDATE_TEMPERATURE,
                                .dataArray { { "temp_c", static_cast<float>(NodeB01::T01_HIGH_TEMPERATURE_C + 1.0F) },
                                             { "pres_hpa", static_cast<int>(1000) },
                                             { "hum_pct", static_cast<int>(100) } } },
                        NodeMsg { .header { .source = NODE_T01, .destArray { NODE_B01 } },
                                .cmdID = UPDATE_DOOR_STATE,
                                .dataArray { { "door_state", static_cast<int>(false) } } },
                        NodeB01::State { .statusLedColor = STATUS_LED_COLOR::GREEN, .isLightON = false,
                                        .isDisplayON = true, .isWarningAudio = true, .isIntrusionAudio = false,
                                        .isAlarmAudio = false, .isMessageToSend = false })
    )
);
