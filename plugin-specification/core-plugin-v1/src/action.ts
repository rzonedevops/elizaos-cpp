import {
  Action as ActionFromTypes,
  Handler as HandlerFromTypes,
  Validator as ValidatorFromTypes,
  ActionExample,
} from './types';
import { fromV2ActionExample, toV2ActionExample } from './actionExample';
import { fromV2State, toV2State } from './state';
import {
  Action as ActionV2,
  Handler as HandlerV2,
  Validator as ValidatorV2,
} from '@elizaos/core-plugin-v2';

/**
 * Action interface for v1 compatibility
 */
export type Action = ActionFromTypes;
export type Handler = HandlerFromTypes;
export type Validator = ValidatorFromTypes;

/**
 * Converts v2 Handler to v1 compatible Handler
 * V2 handlers have additional parameters like responses array
 */
function fromV2Handler(handlerV2: HandlerV2): Handler {
  return async (runtime, message, state, options, callback) => {
    // Convert v1 state to v2 state if provided
    const stateV2 = state ? toV2State(state) : undefined;
    
    try {
      // Call v2 handler with transformed parameters
      // V2 handlers have additional responses parameter, pass empty array
      const responses: any[] = [];
      return await handlerV2(runtime as any, message as any, stateV2 as any, options, callback, responses);
    } catch (error) {
      console.error('Error in v2 handler:', error);
      throw error;
    }
  };
}

/**
 * Converts v1 Handler to v2 Handler
 * V1 handlers don't have responses parameter, so we ignore it
 */
function toV2Handler(handler: Handler): HandlerV2 {
  return async (runtime, message, state, options, callback, responses) => {
    // Convert v2 state to v1 state if provided
    const stateV1 = state ? fromV2State(state as any) : undefined;
    
    try {
      // Call v1 handler, ignoring the responses parameter
      return await handler(runtime as any, message as any, stateV1, options, callback);
    } catch (error) {
      console.error('Error in v1 handler:', error);
      throw error;
    }
  };
}

/**
 * Converts v2 Validator to v1 compatible Validator
 * Both have same signature, so just type cast with runtime conversion
 */
function fromV2Validator(validatorV2: ValidatorV2): Validator {
  return async (runtime, message, state) => {
    const stateV2 = state ? toV2State(state) : undefined;
    return validatorV2(runtime as any, message as any, stateV2 as any);
  };
}

/**
 * Converts v1 Validator to v2 Validator
 * Both have same signature, so just type cast with state conversion
 */
function toV2Validator(validator: Validator): ValidatorV2 {
  return async (runtime, message, state) => {
    const stateV1 = state ? fromV2State(state as any) : undefined;
    return validator(runtime as any, message as any, stateV1);
  };
}

/**
 * Converts v2 Action to v1 compatible Action
 * Main differences:
 * - V1 requires similes array, V2 has optional similes
 * - V1 has optional suppressInitialMessage, V2 doesn't
 * - Handler/Validator signatures differ slightly
 */
export function fromV2Action(actionV2: ActionV2): Action {
  return {
    name: actionV2.name,
    description: actionV2.description,
    similes: actionV2.similes || [], // V1 requires array, V2 is optional
    examples: actionV2.examples ? actionV2.examples.map(exampleGroup => 
      exampleGroup.map(example => fromV2ActionExample(example))
    ) : [],
    handler: fromV2Handler(actionV2.handler),
    validate: fromV2Validator(actionV2.validate),
    // V2 doesn't have suppressInitialMessage, default to false
    suppressInitialMessage: false,
  };
}

/**
 * Converts v1 Action to v2 Action
 * Maps v1 action structure to v2 format
 */
export function toV2Action(action: Action): ActionV2 {
  return {
    name: action.name,
    description: action.description,
    similes: action.similes.length > 0 ? action.similes : undefined, // V2 optional
    examples: action.examples.map(exampleGroup => 
      exampleGroup.map(example => toV2ActionExample(example))
    ),
    handler: toV2Handler(action.handler),
    validate: toV2Validator(action.validate),
    // V2 doesn't have suppressInitialMessage, so it's lost in conversion
  };
}